// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "standard_streams.h"
#include "../ext/utf.h"
#include "mutex.h"
#include "mcfwin.h"

static_assert(sizeof (wchar_t) == sizeof (char16_t), "What?");
static_assert(alignof(wchar_t) == alignof(char16_t), "What?");

#define SAFETY_BUMPER_SIZE       4u
#define STDIN_POPULATION_DELTA   0x1000u
#define STDOUT_FLUSH_THRESHOLD   0x1000u

// _MCFCRT_ASSERT() 会操作标准输入输出流，所以可能会死锁。
#ifdef NDEBUG
#	define SS_ASSERT(expr_)   ((void)((expr_) || (__builtin_unreachable(), 1)))
#else
#	define SS_ASSERT(expr_)   ((void)((expr_) || (__builtin_trap(), 1)))
#endif

typedef struct tagStream {
	HANDLE hPipe;
	bool bConsole;

	_MCFCRT_Mutex vMutex;

	bool bEchoing;
	bool bBuffered;

	void *pStorageBegin;
	void *pDataBegin;
	void *pDataEnd;
	void *pStorageEnd;
} Stream;

static inline void DestroyBuffer(Stream *restrict pStream){
	char *const pchStorageBegin = pStream->pStorageBegin;

	if(pchStorageBegin){
		HeapFree(GetProcessHeap(), 0, pchStorageBegin);
	}

	pStream->pStorageBegin = nullptr;
	pStream->pDataBegin    = nullptr;
	pStream->pDataEnd      = nullptr;
	pStream->pStorageEnd   = nullptr;
}
static inline void Reset(Stream *restrict pStream, HANDLE hPipe, bool bBuffered){
	DestroyBuffer(pStream);

	DWORD dwModes;
	const bool bConsole = GetConsoleMode(hPipe, &dwModes);
	if(!bConsole){
		dwModes = 0;
	}
	const bool bEchoing = dwModes & ENABLE_ECHO_INPUT;

	pStream->hPipe         = hPipe;
	pStream->bConsole      = bConsole;
	pStream->bEchoing      = bEchoing;
	pStream->bBuffered     = bBuffered;
}

static inline HANDLE GetHandle(const Stream *restrict pStream){
	return pStream->hPipe;
}
static inline bool IsConsole(const Stream *restrict pStream){
	return pStream->bConsole;
}

static inline void Lock(Stream *restrict pStream){
	_MCFCRT_WaitForMutexForever(&(pStream->vMutex), _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
}
static inline void Unlock(Stream *restrict pStream){
	_MCFCRT_SignalMutex(&(pStream->vMutex));
}

static inline bool IsEchoing(const Stream *restrict pStream){
	return pStream->bEchoing;
}
static inline DWORD SetEchoing(Stream *restrict pStream, bool bEchoing){
	DWORD dwConsoleMode;
	if(!GetConsoleMode(GetHandle(pStream), &dwConsoleMode)){
		return GetLastError();
	}
	if(bEchoing){
		dwConsoleMode |=  (DWORD)(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);
	} else {
		dwConsoleMode &= ~(DWORD)(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);
	}
	if(!SetConsoleMode(GetHandle(pStream), dwConsoleMode)){
		return GetLastError();
	}
	pStream->bEchoing = bEchoing;
	return 0;
}

static inline bool IsBuffered(const Stream *restrict pStream){
	return pStream->bBuffered;
}
static inline void SetBuffered(Stream *restrict pStream, bool bBuffered){
	pStream->bBuffered = bBuffered;
}

static inline void *GetData(const Stream *restrict pStream){
	char *const pchDataBegin = pStream->pDataBegin;

	return pchDataBegin;
}
static inline size_t GetSize(const Stream *restrict pStream){
	char *const pchDataBegin = pStream->pDataBegin;
	char *const pchDataEnd   = pStream->pDataEnd;

	return (size_t)(pchDataEnd - pchDataBegin);
}
static inline void *GetReservedData(const Stream *restrict pStream){
	char *const pchDataEnd = pStream->pDataEnd;

	return pchDataEnd;
}
static inline size_t GetReservedSize(const Stream *restrict pStream){
	char *const pchDataEnd    = pStream->pDataEnd;
	char *const pchStorageEnd = pStream->pStorageEnd;

	return (size_t)(pchStorageEnd - pchDataEnd);
}

static inline void Discard(Stream *restrict pStream, size_t uSize){
	char *const pchDataBegin = pStream->pDataBegin;
	SS_ASSERT(uSize <= GetSize(pStream));

	pStream->pDataBegin = pchDataBegin + uSize;
}
static inline DWORD ReserveMore(Stream *restrict pStream, size_t uSize){
	char *pchStorageBegin = pStream->pStorageBegin;
	char *pchDataBegin    = pStream->pDataBegin;
	char *pchDataEnd      = pStream->pDataEnd;
	char *pchStorageEnd   = pStream->pStorageEnd;

	if(uSize > GetReservedSize(pStream)){
		const size_t uOldSize = (size_t)(pchDataEnd - pchDataBegin);
		const size_t uMinNewCapacity = uOldSize + uSize;
		if(uMinNewCapacity < uOldSize){
			return ERROR_NOT_ENOUGH_MEMORY;
		}
		const size_t uOldCapacity = (size_t)(pchStorageEnd - pchStorageBegin);
		if(uMinNewCapacity <= uOldCapacity){
			memmove(pchStorageBegin, pchDataBegin, uOldSize);

			pchDataBegin = pchStorageBegin;
			pchDataEnd   = pchDataBegin + uOldSize;

			pStream->pDataBegin = pchDataBegin;
			pStream->pDataEnd   = pchDataEnd;
		} else {
			size_t uNewCapacity = uOldCapacity + 1;
			uNewCapacity += (uNewCapacity >> 1);
			uNewCapacity = (uNewCapacity + 0x0F) & (size_t)-0x10;
			if(uNewCapacity < uMinNewCapacity){
				uNewCapacity = uMinNewCapacity;
			}
			if(uNewCapacity > SIZE_MAX - SAFETY_BUMPER_SIZE){
				return ERROR_NOT_ENOUGH_MEMORY;
			}
			char *const pchNewStorage = HeapAlloc(GetProcessHeap(), 0, uNewCapacity + SAFETY_BUMPER_SIZE);
			if(!pchNewStorage){
				return ERROR_NOT_ENOUGH_MEMORY;
			}
			if(pchStorageBegin){
				memcpy(pchNewStorage, pchDataBegin, uOldSize);
				HeapFree(GetProcessHeap(), 0, pchStorageBegin);
			}
			memset(pchNewStorage + uNewCapacity, 0, SAFETY_BUMPER_SIZE);

			pchStorageBegin = pchNewStorage;
			pchDataBegin    = pchNewStorage;
			pchDataEnd      = pchDataBegin + uOldSize;
			pchStorageEnd   = pchStorageBegin + uNewCapacity;

			pStream->pStorageBegin = pchStorageBegin;
			pStream->pDataBegin    = pchDataBegin;
			pStream->pDataEnd      = pchDataEnd;
			pStream->pStorageEnd   = pchStorageEnd;
		}
	}
	SS_ASSERT(uSize <= GetReservedSize(pStream));
	return 0;
}
static inline void Adopt(Stream *restrict pStream, size_t uSize) _MCFCRT_NOEXCEPT {
	char *const pchDataEnd = pStream->pDataEnd;
	SS_ASSERT(uSize <= GetReservedSize(pStream));

	pStream->pDataEnd = pchDataEnd + uSize;
}

static inline size_t GetUtf8Size(const Stream *restrict pStream){
	const char *const pchData = GetData(pStream);
	size_t uCharsAvail = GetSize(pStream);
	for(unsigned i = 0; i <4; ++i){
		if((uCharsAvail > 0) && (_MCFCRT_GetUtf8CodeUnitType(pchData[uCharsAvail - 1]) == _MCFCRT_kUtf8Trailing)){
			--uCharsAvail;
		} else {
			break;
		}
	}
	return uCharsAvail;
}
static inline size_t GetUtf16Size(const Stream *restrict pStream){
	const wchar_t *const pwcData = GetData(pStream);
	size_t uCharsAvail = GetSize(pStream) / sizeof(char16_t);
	if((uCharsAvail > 0) && (_MCFCRT_GetUtf16CodeUnitType(pwcData[uCharsAvail - 1]) == _MCFCRT_kUtf16LeadingSurrogate)){
		--uCharsAvail;
	}
	return uCharsAvail;
}
static inline DWORD Populate(Stream *restrict pStream, size_t uDelta){
	DWORD dwErrorCode = 0;
	if(!IsBuffered(pStream)){
		dwErrorCode = ERROR_HANDLE_EOF;
	} else {
		dwErrorCode = ReserveMore(pStream, uDelta);
		if(dwErrorCode != 0){
			goto jDone;
		}
		size_t uBytesToRead = GetReservedSize(pStream);
		if(uBytesToRead > UINT32_MAX){
			uBytesToRead = UINT32_MAX;
		}
		if(IsConsole(pStream)){
			const DWORD dwCharsToRead = (DWORD)(uBytesToRead / sizeof(wchar_t));
			if(dwCharsToRead == 0){
				goto jDone;
			}
			DWORD dwCharsRead;
			if(!ReadConsoleW(GetHandle(pStream), GetData(pStream), dwCharsToRead, &dwCharsRead, nullptr) && (dwCharsRead == 0)){
				dwErrorCode = GetLastError();
			}
			Adopt(pStream, dwCharsRead * sizeof(wchar_t));
		} else {
			const DWORD dwBytesToRead = (DWORD)uBytesToRead;
			if(dwBytesToRead == 0){
				goto jDone;
			}
			DWORD dwBytesRead;
			if(!ReadFile(GetHandle(pStream), GetData(pStream), dwBytesToRead, &dwBytesRead, nullptr) && (dwBytesRead == 0)){
				dwErrorCode = GetLastError();
			}
			Adopt(pStream, dwBytesRead);
		}
	}
jDone:
	return dwErrorCode;
}
static inline DWORD Flush(Stream *restrict pStream, size_t uThreshold){
	DWORD dwErrorCode = 0;
	for(;;){
		size_t uBytesToWrite = GetSize(pStream);
		if(uBytesToWrite < uThreshold){
			goto jDone;
		}
		if(uBytesToWrite > UINT32_MAX){
			uBytesToWrite = UINT32_MAX;
		}
		if(IsConsole(pStream)){
			const DWORD dwCharsToWrite = (DWORD)(uBytesToWrite / sizeof(wchar_t));
			if(dwCharsToWrite == 0){
				goto jDone;
			}
			DWORD dwCharsWritten;
			if(!WriteConsoleW(GetHandle(pStream), GetData(pStream), dwCharsToWrite, &dwCharsWritten, nullptr) && (dwCharsWritten == 0)){
				dwErrorCode = GetLastError();
			}
			Discard(pStream, dwCharsWritten * sizeof(wchar_t));
		} else {
			const DWORD dwBytesToWrite = (DWORD)uBytesToWrite;
			if(dwBytesToWrite == 0){
				goto jDone;
			}
			DWORD dwCharsWritten;
			if(!WriteFile(GetHandle(pStream), GetData(pStream), dwBytesToWrite, &dwCharsWritten, nullptr) && (dwCharsWritten == 0)){
				dwErrorCode = GetLastError();
			}
			Discard(pStream, dwCharsWritten);
		}
	}
jDone:
	return dwErrorCode;
}
static inline DWORD FlushSystemBuffers(Stream *restrict pStream){
	DWORD dwErrorCode = 0;
	if(!IsConsole(pStream)){
		if(!FlushSystemBuffers(GetHandle(pStream))){
			return GetLastError();
		}
	}
	return dwErrorCode;
}

static inline bool IsEof(char32_t c32CodePoint){
	if(c32CodePoint == 4){ // Ctrl-D
		return true;
	}
	if(c32CodePoint == 26){ // Ctrl-Z
		return true;
	}
	return false;
}

static Stream g_vStdIn, g_vStdOut, g_vStdErr;

bool __MCFCRT_StandardStreamsInit(void){
	Reset(&g_vStdErr, GetStdHandle(STD_ERROR_HANDLE),  false);
	Reset(&g_vStdOut, GetStdHandle(STD_OUTPUT_HANDLE), true);
	Reset(&g_vStdIn,  GetStdHandle(STD_INPUT_HANDLE),  true);
	return true;
}
void __MCFCRT_StandardStreamsUninit(void){
	Flush(&g_vStdOut, 0);
	Flush(&g_vStdErr, 0);

	Reset(&g_vStdIn,  nullptr, false);
	Reset(&g_vStdOut, nullptr, false);
	Reset(&g_vStdErr, nullptr, false);
}

int _MCFCRT_PeekStandardInputByte(void){
	_MCFCRT_FlushStandardOutput(false);

	int nRead = -1;
	DWORD dwErrorCode = 0;
	if(GetHandle(&g_vStdIn)){
		Lock(&g_vStdIn);
		{
			if(IsConsole(&g_vStdIn)){
				size_t uCharsAvail = GetUtf16Size(&g_vStdIn);
				if(uCharsAvail == 0){
					dwErrorCode = Populate(&g_vStdIn, STDIN_POPULATION_DELTA);
					if(dwErrorCode != 0){
						goto jDone;
					}
					uCharsAvail = GetUtf16Size(&g_vStdIn);
					if(uCharsAvail == 0){
						dwErrorCode = ERROR_HANDLE_EOF;
						goto jDone;
					}
				}
				const char16_t *const pc16ReadBegin = GetData(&g_vStdIn);
				const char16_t *pc16Read = pc16ReadBegin;
				const char32_t c32CodePoint = _MCFCRT_DecodeUtf16(&pc16Read);
				if(IsEof(c32CodePoint)){
					SetBuffered(&g_vStdIn, false);
					DestroyBuffer(&g_vStdIn);
					dwErrorCode = ERROR_HANDLE_EOF;
					goto jDone;
				}
				nRead = (int)(c32CodePoint & 0x00FF);
			} else {
				size_t uBytesAvail = GetSize(&g_vStdIn);
				if(uBytesAvail == 0){
					dwErrorCode = Populate(&g_vStdIn, STDIN_POPULATION_DELTA);
					if(dwErrorCode != 0){
						goto jDone;
					}
					uBytesAvail = GetSize(&g_vStdIn);
					if(uBytesAvail == 0){
						dwErrorCode = ERROR_HANDLE_EOF;
						goto jDone;
					}
				}
				const unsigned char *const pbyReadBegin = GetData(&g_vStdIn);
				const unsigned char *pbyRead = pbyReadBegin;
				nRead = *(pbyRead++);
			}
		}
	jDone:
		Unlock(&g_vStdIn);
	} else {
		dwErrorCode = ERROR_BROKEN_PIPE;
	}
	if(nRead < 0){
		SetLastError(dwErrorCode);
	}
	return nRead;
}
int _MCFCRT_ReadStandardInputByte(void){
	_MCFCRT_FlushStandardOutput(false);

	int nRead = -1;
	DWORD dwErrorCode = 0;
	if(GetHandle(&g_vStdIn)){
		Lock(&g_vStdIn);
		{
			if(IsConsole(&g_vStdIn)){
				size_t uCharsAvail = GetUtf16Size(&g_vStdIn);
				if(uCharsAvail == 0){
					dwErrorCode = Populate(&g_vStdIn, STDIN_POPULATION_DELTA);
					if(dwErrorCode != 0){
						goto jDone;
					}
					uCharsAvail = GetUtf16Size(&g_vStdIn);
					if(uCharsAvail == 0){
						dwErrorCode = ERROR_HANDLE_EOF;
						goto jDone;
					}
				}
				const char16_t *const pc16ReadBegin = GetData(&g_vStdIn);
				const char16_t *pc16Read = pc16ReadBegin;
				const char32_t c32CodePoint = _MCFCRT_DecodeUtf16(&pc16Read);
				if(IsEof(c32CodePoint)){
					SetBuffered(&g_vStdIn, false);
					DestroyBuffer(&g_vStdIn);
					dwErrorCode = ERROR_HANDLE_EOF;
					goto jDone;
				}
				nRead = (int)(c32CodePoint & 0x00FF);
				Discard(&g_vStdIn, (size_t)((const char *)pc16Read - (const char *)pc16ReadBegin));
			} else {
				size_t uBytesAvail = GetSize(&g_vStdIn);
				if(uBytesAvail == 0){
					dwErrorCode = Populate(&g_vStdIn, STDIN_POPULATION_DELTA);
					if(dwErrorCode != 0){
						goto jDone;
					}
					uBytesAvail = GetSize(&g_vStdIn);
					if(uBytesAvail == 0){
						dwErrorCode = ERROR_HANDLE_EOF;
						goto jDone;
					}
				}
				const unsigned char *const pbyReadBegin = GetData(&g_vStdIn);
				const unsigned char *pbyRead = pbyReadBegin;
				nRead = *(pbyRead++);
				Discard(&g_vStdIn, (size_t)(pbyRead - pbyReadBegin));
			}
		}
	jDone:
		Unlock(&g_vStdIn);
	} else {
		dwErrorCode = ERROR_BROKEN_PIPE;
	}
	if(nRead < 0){
		SetLastError(dwErrorCode);
	}
	return nRead;
}
size_t _MCFCRT_PeekStandardInputBinary(void *restrict pData, size_t uSize){
	_MCFCRT_FlushStandardOutput(false);

	size_t uBytesCopied = 0;
	DWORD dwErrorCode = 0;
	if(GetHandle(&g_vStdIn)){
		Lock(&g_vStdIn);
		{
			if(IsConsole(&g_vStdIn)){
				size_t uCharsAvail = GetUtf16Size(&g_vStdIn);
				if(uCharsAvail == 0){
					dwErrorCode = Populate(&g_vStdIn, STDIN_POPULATION_DELTA);
					if(dwErrorCode != 0){
						goto jDone;
					}
					uCharsAvail = GetUtf16Size(&g_vStdIn);
					if(uCharsAvail == 0){
						dwErrorCode = ERROR_HANDLE_EOF;
						goto jDone;
					}
				}
				char *const pchWriteBegin = pData;
				const char16_t *const pc16ReadBegin = GetData(&g_vStdIn);
				const char16_t *pc16Read = pc16ReadBegin;
				for(;;){
					char achDecoded[4];
					char *pchDecodedEnd = achDecoded;
					const char16_t *pc16ReadNext = pc16Read;
					const char32_t c32CodePoint = _MCFCRT_EncodeUtf8FromUtf16(&pchDecodedEnd, &pc16ReadNext);
					if(IsEof(c32CodePoint)){
						SetBuffered(&g_vStdIn, false);
						if(pc16Read == pc16ReadBegin){
							DestroyBuffer(&g_vStdIn);
							dwErrorCode = ERROR_HANDLE_EOF;
							goto jDone;
						}
						break;
					}
					if((size_t)(pc16ReadNext - pc16ReadBegin) > uCharsAvail){
						dwErrorCode = ERROR_HANDLE_EOF;
						break;
					}
					const size_t uBytesDecoded = (size_t)(pchDecodedEnd - achDecoded);
					if(uBytesDecoded > uSize - uBytesCopied){
						break;
					}
					memcpy(pchWriteBegin + uBytesCopied, achDecoded, uBytesDecoded);
					uBytesCopied += uBytesDecoded;
					pc16Read = pc16ReadNext;
				}
			} else {
				size_t uBytesAvail = GetSize(&g_vStdIn);
				if(uBytesAvail == 0){
					dwErrorCode = Populate(&g_vStdIn, STDIN_POPULATION_DELTA);
					if(dwErrorCode != 0){
						goto jDone;
					}
					uBytesAvail = GetSize(&g_vStdIn);
					if(uBytesAvail == 0){
						dwErrorCode = ERROR_HANDLE_EOF;
						goto jDone;
					}
				}
				unsigned char *const pbyWriteBegin = pData;
				const unsigned char *const pbyReadBegin = GetData(&g_vStdIn);
				const unsigned char *pbyRead = pbyReadBegin;
				size_t uBytesToCopy = uSize;
				if(uBytesToCopy > uBytesAvail){
					uBytesToCopy = uBytesAvail;
				}
				memcpy(pbyWriteBegin + uBytesCopied, pbyRead, uBytesToCopy);
				uBytesCopied += uBytesToCopy;
				pbyRead += uBytesToCopy;
			}
		}
	jDone:
		Unlock(&g_vStdIn);
	} else {
		dwErrorCode = ERROR_BROKEN_PIPE;
	}
	if(uBytesCopied == 0){
		SetLastError(dwErrorCode);
	}
	return uBytesCopied;
}
size_t _MCFCRT_ReadStandardInputBinary(void *restrict pData, size_t uSize){
	_MCFCRT_FlushStandardOutput(false);

	size_t uBytesCopied = 0;
	DWORD dwErrorCode = 0;
	if(GetHandle(&g_vStdIn)){
		Lock(&g_vStdIn);
		{
			if(IsConsole(&g_vStdIn)){
				size_t uCharsAvail = GetUtf16Size(&g_vStdIn);
				if(uCharsAvail == 0){
					dwErrorCode = Populate(&g_vStdIn, STDIN_POPULATION_DELTA);
					if(dwErrorCode != 0){
						goto jDone;
					}
					uCharsAvail = GetUtf16Size(&g_vStdIn);
					if(uCharsAvail == 0){
						dwErrorCode = ERROR_HANDLE_EOF;
						goto jDone;
					}
				}
				char *const pchWriteBegin = pData;
				const char16_t *const pc16ReadBegin = GetData(&g_vStdIn);
				const char16_t *pc16Read = pc16ReadBegin;
				for(;;){
					char achDecoded[4];
					char *pchDecodedEnd = achDecoded;
					const char16_t *pc16ReadNext = pc16Read;
					const char32_t c32CodePoint = _MCFCRT_EncodeUtf8FromUtf16(&pchDecodedEnd, &pc16ReadNext);
					if(IsEof(c32CodePoint)){
						SetBuffered(&g_vStdIn, false);
						if(pc16Read == pc16ReadBegin){
							DestroyBuffer(&g_vStdIn);
							dwErrorCode = ERROR_HANDLE_EOF;
							goto jDone;
						}
						break;
					}
					if((size_t)(pc16ReadNext - pc16ReadBegin) > uCharsAvail){
						dwErrorCode = ERROR_HANDLE_EOF;
						break;
					}
					const size_t uBytesDecoded = (size_t)(pchDecodedEnd - achDecoded);
					if(uBytesDecoded > uSize - uBytesCopied){
						break;
					}
					memcpy(pchWriteBegin + uBytesCopied, achDecoded, uBytesDecoded);
					uBytesCopied += uBytesDecoded;
					pc16Read = pc16ReadNext;
				}
				Discard(&g_vStdIn, (size_t)((const char *)pc16Read - (const char *)pc16ReadBegin));
			} else {
				size_t uBytesAvail = GetSize(&g_vStdIn);
				if(uBytesAvail == 0){
					dwErrorCode = Populate(&g_vStdIn, STDIN_POPULATION_DELTA);
					if(dwErrorCode != 0){
						goto jDone;
					}
					uBytesAvail = GetSize(&g_vStdIn);
					if(uBytesAvail == 0){
						dwErrorCode = ERROR_HANDLE_EOF;
						goto jDone;
					}
				}
				unsigned char *const pbyWriteBegin = pData;
				const unsigned char *const pbyReadBegin = GetData(&g_vStdIn);
				const unsigned char *pbyRead = pbyReadBegin;
				size_t uBytesToCopy = uSize;
				if(uBytesToCopy > uBytesAvail){
					uBytesToCopy = uBytesAvail;
				}
				memcpy(pbyWriteBegin + uBytesCopied, pbyRead, uBytesToCopy);
				uBytesCopied += uBytesToCopy;
				pbyRead += uBytesToCopy;
				Discard(&g_vStdIn, (size_t)(pbyRead - pbyReadBegin));
			}
		}
	jDone:
		Unlock(&g_vStdIn);
	} else {
		dwErrorCode = ERROR_BROKEN_PIPE;
	}
	if(uBytesCopied == 0){
		SetLastError(dwErrorCode);
	}
	return uBytesCopied;
}
size_t _MCFCRT_DiscardStandardInputBinary(size_t uSize){
	_MCFCRT_FlushStandardOutput(false);

	size_t uBytesCopied = 0;
	DWORD dwErrorCode = 0;
	if(GetHandle(&g_vStdIn)){
		Lock(&g_vStdIn);
		{
			if(IsConsole(&g_vStdIn)){
				size_t uCharsAvail = GetUtf16Size(&g_vStdIn);
				if(uCharsAvail == 0){
					dwErrorCode = Populate(&g_vStdIn, STDIN_POPULATION_DELTA);
					if(dwErrorCode != 0){
						goto jDone;
					}
					uCharsAvail = GetUtf16Size(&g_vStdIn);
					if(uCharsAvail == 0){
						dwErrorCode = ERROR_HANDLE_EOF;
						goto jDone;
					}
				}
				const char16_t *const pc16ReadBegin = GetData(&g_vStdIn);
				const char16_t *pc16Read = pc16ReadBegin;
				for(;;){
					char achDecoded[4];
					char *pchDecodedEnd = achDecoded;
					const char16_t *pc16ReadNext = pc16Read;
					const char32_t c32CodePoint = _MCFCRT_EncodeUtf8FromUtf16(&pchDecodedEnd, &pc16ReadNext);
					if(IsEof(c32CodePoint)){
						SetBuffered(&g_vStdIn, false);
						if(pc16Read == pc16ReadBegin){
							DestroyBuffer(&g_vStdIn);
							dwErrorCode = ERROR_HANDLE_EOF;
							goto jDone;
						}
						break;
					}
					if((size_t)(pc16ReadNext - pc16ReadBegin) > uCharsAvail){
						dwErrorCode = ERROR_HANDLE_EOF;
						break;
					}
					const size_t uBytesDecoded = (size_t)(pchDecodedEnd - achDecoded);
					if(uBytesDecoded > uSize - uBytesCopied){
						break;
					}
					uBytesCopied += uBytesDecoded;
					pc16Read = pc16ReadNext;
				}
				Discard(&g_vStdIn, (size_t)((const char *)pc16Read - (const char *)pc16ReadBegin));
			} else {
				size_t uBytesAvail = GetSize(&g_vStdIn);
				if(uBytesAvail == 0){
					dwErrorCode = Populate(&g_vStdIn, STDIN_POPULATION_DELTA);
					if(dwErrorCode != 0){
						goto jDone;
					}
					uBytesAvail = GetSize(&g_vStdIn);
					if(uBytesAvail == 0){
						dwErrorCode = ERROR_HANDLE_EOF;
						goto jDone;
					}
				}
				const unsigned char *const pbyReadBegin = GetData(&g_vStdIn);
				const unsigned char *pbyRead = pbyReadBegin;
				size_t uBytesToCopy = uSize;
				if(uBytesToCopy > uBytesAvail){
					uBytesToCopy = uBytesAvail;
				}
				uBytesCopied += uBytesToCopy;
				pbyRead += uBytesToCopy;
				Discard(&g_vStdIn, (size_t)(pbyRead - pbyReadBegin));
			}
		}
	jDone:
		Unlock(&g_vStdIn);
	} else {
		dwErrorCode = ERROR_BROKEN_PIPE;
	}
	if(uBytesCopied == 0){
		SetLastError(dwErrorCode);
	}
	return uBytesCopied;
}
long _MCFCRT_PeekStandardInputChar32(void){
	_MCFCRT_FlushStandardOutput(false);

	long lRead = -1;
	DWORD dwErrorCode = 0;
	if(GetHandle(&g_vStdIn)){
		Lock(&g_vStdIn);
		{
			if(IsConsole(&g_vStdIn)){
				size_t uCharsAvail = GetUtf16Size(&g_vStdIn);
				if(uCharsAvail == 0){
					dwErrorCode = Populate(&g_vStdIn, STDIN_POPULATION_DELTA);
					if(dwErrorCode != 0){
						goto jDone;
					}
					uCharsAvail = GetUtf16Size(&g_vStdIn);
					if(uCharsAvail == 0){
						dwErrorCode = ERROR_HANDLE_EOF;
						goto jDone;
					}
				}
				const char16_t *const pc16ReadBegin = GetData(&g_vStdIn);
				const char16_t *pc16Read = pc16ReadBegin;
				const char32_t c32CodePoint = _MCFCRT_DecodeUtf16(&pc16Read);
				if((size_t)(pc16Read - pc16ReadBegin) > uCharsAvail){
					dwErrorCode = ERROR_HANDLE_EOF;
					goto jDone;
				}
				if(IsEof(c32CodePoint)){
					SetBuffered(&g_vStdIn, false);
					DestroyBuffer(&g_vStdIn);
					dwErrorCode = ERROR_HANDLE_EOF;
					goto jDone;
				}
				lRead = (long)c32CodePoint;
			} else {
				size_t uBytesAvail = GetUtf8Size(&g_vStdIn);
				if(uBytesAvail == 0){
					dwErrorCode = Populate(&g_vStdIn, STDIN_POPULATION_DELTA);
					if(dwErrorCode != 0){
						goto jDone;
					}
					uBytesAvail = GetUtf8Size(&g_vStdIn);
					if(uBytesAvail == 0){
						dwErrorCode = ERROR_HANDLE_EOF;
						goto jDone;
					}
				}
				const char *const pchReadBegin = GetData(&g_vStdIn);
				const char *pchRead = pchReadBegin;
				const char32_t c32CodePoint = _MCFCRT_DecodeUtf8(&pchRead);
				if((size_t)(pchRead - pchReadBegin) > uBytesAvail){
					dwErrorCode = ERROR_HANDLE_EOF;
					goto jDone;
				}
				lRead = (long)c32CodePoint;
			}
		}
	jDone:
		Unlock(&g_vStdIn);
	} else {
		dwErrorCode = ERROR_BROKEN_PIPE;
	}
	if(lRead < 0){
		SetLastError(dwErrorCode);
	}
	return lRead;
}
long _MCFCRT_ReadStandardInputChar32(void){
	_MCFCRT_FlushStandardOutput(false);

	long lRead = -1;
	DWORD dwErrorCode = 0;
	if(GetHandle(&g_vStdIn)){
		Lock(&g_vStdIn);
		{
			if(IsConsole(&g_vStdIn)){
				size_t uCharsAvail = GetUtf16Size(&g_vStdIn);
				if(uCharsAvail == 0){
					dwErrorCode = Populate(&g_vStdIn, STDIN_POPULATION_DELTA);
					if(dwErrorCode != 0){
						goto jDone;
					}
					uCharsAvail = GetUtf16Size(&g_vStdIn);
					if(uCharsAvail == 0){
						dwErrorCode = ERROR_HANDLE_EOF;
						goto jDone;
					}
				}
				const char16_t *const pc16ReadBegin = GetData(&g_vStdIn);
				const char16_t *pc16Read = pc16ReadBegin;
				const char32_t c32CodePoint = _MCFCRT_DecodeUtf16(&pc16Read);
				if((size_t)(pc16Read - pc16ReadBegin) > uCharsAvail){
					dwErrorCode = ERROR_HANDLE_EOF;
					goto jDone;
				}
				if(IsEof(c32CodePoint)){
					SetBuffered(&g_vStdIn, false);
					DestroyBuffer(&g_vStdIn);
					dwErrorCode = ERROR_HANDLE_EOF;
					goto jDone;
				}
				lRead = (long)c32CodePoint;
				Discard(&g_vStdIn, (size_t)((const char *)pc16Read - (const char *)pc16ReadBegin));
			} else {
				size_t uBytesAvail = GetUtf8Size(&g_vStdIn);
				if(uBytesAvail == 0){
					dwErrorCode = Populate(&g_vStdIn, STDIN_POPULATION_DELTA);
					if(dwErrorCode != 0){
						goto jDone;
					}
					uBytesAvail = GetUtf8Size(&g_vStdIn);
					if(uBytesAvail == 0){
						dwErrorCode = ERROR_HANDLE_EOF;
						goto jDone;
					}
				}
				const char *const pchReadBegin = GetData(&g_vStdIn);
				const char *pchRead = pchReadBegin;
				const char32_t c32CodePoint = _MCFCRT_DecodeUtf8(&pchRead);
				if((size_t)(pchRead - pchReadBegin) > uBytesAvail){
					dwErrorCode = ERROR_HANDLE_EOF;
					goto jDone;
				}
				lRead = (long)c32CodePoint;
				Discard(&g_vStdIn, (size_t)(pchRead - pchReadBegin));
			}
		}
	jDone:
		Unlock(&g_vStdIn);
	} else {
		dwErrorCode = ERROR_BROKEN_PIPE;
	}
	if(lRead < 0){
		SetLastError(dwErrorCode);
	}
	return lRead;
}
size_t _MCFCRT_PeekStandardInputText(wchar_t *restrict pwcText, size_t uLength, bool bSingleLine){
	_MCFCRT_FlushStandardOutput(false);

	size_t uCharsCopied = 0;
	DWORD dwErrorCode = 0;
	if(GetHandle(&g_vStdIn)){
		Lock(&g_vStdIn);
		{
			if(IsConsole(&g_vStdIn)){
				size_t uCharsAvail = GetUtf16Size(&g_vStdIn);
				if(uCharsAvail == 0){
					dwErrorCode = Populate(&g_vStdIn, STDIN_POPULATION_DELTA);
					if(dwErrorCode != 0){
						goto jDone;
					}
					uCharsAvail = GetUtf16Size(&g_vStdIn);
					if(uCharsAvail == 0){
						dwErrorCode = ERROR_HANDLE_EOF;
						goto jDone;
					}
				}
				char16_t *const pc16WriteBegin = (void *)pwcText;
				const char16_t *const pc16ReadBegin = GetData(&g_vStdIn);
				const char16_t *pc16Read = pc16ReadBegin;
				size_t uCharsToCopy = uCharsAvail;
				if(uCharsToCopy > uLength){
					uCharsToCopy = uLength;
				}
				if(bSingleLine){
					for(size_t i = 0; i < uCharsToCopy; ++i){
						if(pc16Read[i] == u'\n'){
							uCharsToCopy = i + 1;
							break;
						}
					}
				}
				memcpy(pc16WriteBegin + uCharsCopied, pc16Read, uCharsToCopy * sizeof(char16_t));
				uCharsCopied += uCharsToCopy;
				pc16Read += uCharsToCopy;
			} else {
				size_t uBytesAvail = GetUtf8Size(&g_vStdIn);
				if(uBytesAvail == 0){
					dwErrorCode = Populate(&g_vStdIn, STDIN_POPULATION_DELTA);
					if(dwErrorCode != 0){
						goto jDone;
					}
					uBytesAvail = GetSize(&g_vStdIn);
					if(uBytesAvail == 0){
						dwErrorCode = ERROR_HANDLE_EOF;
						goto jDone;
					}
				}
				char16_t *const pc16WriteBegin = (void *)pwcText;
				const char *const pchReadBegin = GetData(&g_vStdIn);
				const char *pchRead = pchReadBegin;
				for(;;){
					char16_t ac16Encoded[2];
					char16_t *pc16EncodedEnd = ac16Encoded;
					const char *pchReadNext = pchRead;
					const char32_t c32CodePoint = _MCFCRT_EncodeUtf16FromUtf8(&pc16EncodedEnd, &pchReadNext);
					if((size_t)(pchReadNext - pchReadBegin) > uBytesAvail){
						dwErrorCode = ERROR_HANDLE_EOF;
						break;
					}
					const size_t uCharsEncoded = (size_t)(pc16EncodedEnd - ac16Encoded);
					if(uCharsEncoded > uLength - uCharsCopied){
						break;
					}
					memcpy(pc16WriteBegin + uCharsCopied, ac16Encoded, uCharsEncoded * sizeof(char16_t));
					uCharsCopied += uCharsEncoded;
					pchRead = pchReadNext;
					if(bSingleLine){
						if(c32CodePoint == U'\n'){
							break;
						}
					}
				}
			}
		}
	jDone:
		Unlock(&g_vStdIn);
	} else {
		dwErrorCode = ERROR_BROKEN_PIPE;
	}
	if(uCharsCopied == 0){
		SetLastError(dwErrorCode);
	}
	return uCharsCopied;
}
size_t _MCFCRT_ReadStandardInputText(wchar_t *restrict pwcText, size_t uLength, bool bSingleLine){
	_MCFCRT_FlushStandardOutput(false);

	size_t uCharsCopied = 0;
	DWORD dwErrorCode = 0;
	if(GetHandle(&g_vStdIn)){
		Lock(&g_vStdIn);
		{
			if(IsConsole(&g_vStdIn)){
				size_t uCharsAvail = GetUtf16Size(&g_vStdIn);
				if(uCharsAvail == 0){
					dwErrorCode = Populate(&g_vStdIn, STDIN_POPULATION_DELTA);
					if(dwErrorCode != 0){
						goto jDone;
					}
					uCharsAvail = GetUtf16Size(&g_vStdIn);
					if(uCharsAvail == 0){
						dwErrorCode = ERROR_HANDLE_EOF;
						goto jDone;
					}
				}
				char16_t *const pc16WriteBegin = (void *)pwcText;
				const char16_t *const pc16ReadBegin = GetData(&g_vStdIn);
				const char16_t *pc16Read = pc16ReadBegin;
				size_t uCharsToCopy = uCharsAvail;
				if(uCharsToCopy > uLength){
					uCharsToCopy = uLength;
				}
				if(bSingleLine){
					for(size_t i = 0; i < uCharsToCopy; ++i){
						if(pc16Read[i] == u'\n'){
							uCharsToCopy = i + 1;
							break;
						}
					}
				}
				memcpy(pc16WriteBegin + uCharsCopied, pc16Read, uCharsToCopy * sizeof(char16_t));
				uCharsCopied += uCharsToCopy;
				pc16Read += uCharsToCopy;
				Discard(&g_vStdIn, (size_t)((const char *)pc16Read - (const char *)pc16ReadBegin));
			} else {
				size_t uBytesAvail = GetUtf8Size(&g_vStdIn);
				if(uBytesAvail == 0){
					dwErrorCode = Populate(&g_vStdIn, STDIN_POPULATION_DELTA);
					if(dwErrorCode != 0){
						goto jDone;
					}
					uBytesAvail = GetSize(&g_vStdIn);
					if(uBytesAvail == 0){
						dwErrorCode = ERROR_HANDLE_EOF;
						goto jDone;
					}
				}
				char16_t *const pc16WriteBegin = (void *)pwcText;
				const char *const pchReadBegin = GetData(&g_vStdIn);
				const char *pchRead = pchReadBegin;
				for(;;){
					char16_t ac16Encoded[2];
					char16_t *pc16EncodedEnd = ac16Encoded;
					const char *pchReadNext = pchRead;
					const char32_t c32CodePoint = _MCFCRT_EncodeUtf16FromUtf8(&pc16EncodedEnd, &pchReadNext);
					if((size_t)(pchReadNext - pchReadBegin) > uBytesAvail){
						dwErrorCode = ERROR_HANDLE_EOF;
						break;
					}
					const size_t uCharsEncoded = (size_t)(pc16EncodedEnd - ac16Encoded);
					if(uCharsEncoded > uLength - uCharsCopied){
						break;
					}
					memcpy(pc16WriteBegin + uCharsCopied, ac16Encoded, uCharsEncoded * sizeof(char16_t));
					uCharsCopied += uCharsEncoded;
					pchRead = pchReadNext;
					if(bSingleLine){
						if(c32CodePoint == U'\n'){
							break;
						}
					}
				}
				Discard(&g_vStdIn, (size_t)(pchRead - pchReadBegin));
			}
		}
	jDone:
		Unlock(&g_vStdIn);
	} else {
		dwErrorCode = ERROR_BROKEN_PIPE;
	}
	if(uCharsCopied == 0){
		SetLastError(dwErrorCode);
	}
	return uCharsCopied;
}
size_t _MCFCRT_DiscardStandardInputText(size_t uLength, bool bSingleLine){
	_MCFCRT_FlushStandardOutput(false);

	size_t uCharsCopied = 0;
	DWORD dwErrorCode = 0;
	if(GetHandle(&g_vStdIn)){
		Lock(&g_vStdIn);
		{
			if(IsConsole(&g_vStdIn)){
				size_t uCharsAvail = GetUtf16Size(&g_vStdIn);
				if(uCharsAvail == 0){
					dwErrorCode = Populate(&g_vStdIn, STDIN_POPULATION_DELTA);
					if(dwErrorCode != 0){
						goto jDone;
					}
					uCharsAvail = GetUtf16Size(&g_vStdIn);
					if(uCharsAvail == 0){
						dwErrorCode = ERROR_HANDLE_EOF;
						goto jDone;
					}
				}
				const char16_t *const pc16ReadBegin = GetData(&g_vStdIn);
				const char16_t *pc16Read = pc16ReadBegin;
				size_t uCharsToCopy = uCharsAvail;
				if(uCharsToCopy > uLength){
					uCharsToCopy = uLength;
				}
				if(bSingleLine){
					for(size_t i = 0; i < uCharsToCopy; ++i){
						if(pc16Read[i] == u'\n'){
							uCharsToCopy = i + 1;
							break;
						}
					}
				}
				uCharsCopied += uCharsToCopy;
				pc16Read += uCharsToCopy;
				Discard(&g_vStdIn, (size_t)((const char *)pc16Read - (const char *)pc16ReadBegin));
			} else {
				size_t uBytesAvail = GetUtf8Size(&g_vStdIn);
				if(uBytesAvail == 0){
					dwErrorCode = Populate(&g_vStdIn, STDIN_POPULATION_DELTA);
					if(dwErrorCode != 0){
						goto jDone;
					}
					uBytesAvail = GetSize(&g_vStdIn);
					if(uBytesAvail == 0){
						dwErrorCode = ERROR_HANDLE_EOF;
						goto jDone;
					}
				}
				const char *const pchReadBegin = GetData(&g_vStdIn);
				const char *pchRead = pchReadBegin;
				for(;;){
					char16_t ac16Encoded[2];
					char16_t *pc16EncodedEnd = ac16Encoded;
					const char *pchReadNext = pchRead;
					const char32_t c32CodePoint = _MCFCRT_EncodeUtf16FromUtf8(&pc16EncodedEnd, &pchReadNext);
					if((size_t)(pchReadNext - pchReadBegin) > uBytesAvail){
						dwErrorCode = ERROR_HANDLE_EOF;
						break;
					}
					const size_t uCharsEncoded = (size_t)(pc16EncodedEnd - ac16Encoded);
					if(uCharsEncoded > uLength - uCharsCopied){
						break;
					}
					uCharsCopied += uCharsEncoded;
					pchRead = pchReadNext;
					if(bSingleLine){
						if(c32CodePoint == U'\n'){
							break;
						}
					}
				}
				Discard(&g_vStdIn, (size_t)(pchRead - pchReadBegin));
			}
		}
	jDone:
		Unlock(&g_vStdIn);
	} else {
		dwErrorCode = ERROR_BROKEN_PIPE;
	}
	if(uCharsCopied == 0){
		SetLastError(dwErrorCode);
	}
	return uCharsCopied;
}
bool _MCFCRT_IsStandardInputEchoing(void){
	bool bEchoing;
	if(GetHandle(&g_vStdOut)){
		Lock(&g_vStdIn);
		{
			bEchoing = IsEchoing(&g_vStdIn);
		}
		Unlock(&g_vStdIn);
	} else {
		bEchoing = false;
	}
	return bEchoing;
}
bool _MCFCRT_SetStandardInputEchoing(bool bEchoing){
	bool bSuccess = false;
	DWORD dwErrorCode = 0;
	if(GetHandle(&g_vStdOut)){
		Lock(&g_vStdIn);
		{
			dwErrorCode = SetEchoing(&g_vStdIn, bEchoing);
			if(dwErrorCode != 0){
				goto jDone;
			}
			bSuccess = true;
		}
	jDone:
		Unlock(&g_vStdIn);
	} else {
		dwErrorCode = ERROR_BROKEN_PIPE;
	}
	if(!bSuccess){
		SetLastError(dwErrorCode);
	}
	return bSuccess;
}

bool _MCFCRT_WriteStandardOutputByte(unsigned char byData){
	bool bSuccess = false;
	DWORD dwErrorCode = 0;
	if(GetHandle(&g_vStdOut)){
		Lock(&g_vStdOut);
		{
			if(IsConsole(&g_vStdOut)){
				dwErrorCode = ReserveMore(&g_vStdOut, sizeof(char16_t));
				if(dwErrorCode != 0){
					goto jDone;
				}
				char16_t *const pc16WriteBegin = GetReservedData(&g_vStdOut);
				char16_t *pc16Write = pc16WriteBegin;
				*(pc16Write++) = (char16_t)(byData & 0x00FF);
				Adopt(&g_vStdOut, (size_t)((char *)pc16Write - (char *)pc16WriteBegin));
				bSuccess = true;
			} else {
				dwErrorCode = ReserveMore(&g_vStdOut, 1);
				if(dwErrorCode != 0){
					goto jDone;
				}
				unsigned char *const pbyWriteBegin = GetReservedData(&g_vStdOut);
				unsigned char *pbyWrite = pbyWriteBegin;
				*(pbyWrite++) = byData;
				Adopt(&g_vStdOut, (size_t)(pbyWrite - pbyWriteBegin));
				bSuccess = true;
			}
		}
	jDone:
		if(IsBuffered(&g_vStdOut)){
			Flush(&g_vStdOut, STDOUT_FLUSH_THRESHOLD);
		} else {
			Flush(&g_vStdOut, 0);
		}
		Unlock(&g_vStdOut);
	} else {
		dwErrorCode = ERROR_BROKEN_PIPE;
	}
	if(!bSuccess){
		SetLastError(dwErrorCode);
	}
	return bSuccess;
}
bool _MCFCRT_WriteStandardOutputBinary(const void *restrict pData, size_t uSize){
	bool bSuccess = false;
	DWORD dwErrorCode = 0;
	if(GetHandle(&g_vStdOut)){
		Lock(&g_vStdOut);
		{
			if(IsConsole(&g_vStdOut)){
				// UTF-16 <= UTF-8: 1:1 (max), 1:2, 1:3, 2:4
				if(uSize > SIZE_MAX / sizeof(char16_t)){
					dwErrorCode = ERROR_NOT_ENOUGH_MEMORY;
					goto jDone;
				}
				dwErrorCode = ReserveMore(&g_vStdOut, uSize * sizeof(char16_t));
				if(dwErrorCode != 0){
					goto jDone;
				}
				char16_t *const pc16WriteBegin = GetReservedData(&g_vStdOut);
				char16_t *pc16Write = pc16WriteBegin;
				const char *pchRead = pData;
				const char *const pchReadEnd = pchRead + uSize;
				while(pchRead < pchReadEnd){
					_MCFCRT_EncodeUtf16FromUtf8(&pc16Write, &pchRead);
				}
				Adopt(&g_vStdOut, (size_t)((char *)pc16Write - (char *)pc16WriteBegin));
				bSuccess = true;
			} else {
				dwErrorCode = ReserveMore(&g_vStdOut, uSize);
				if(dwErrorCode != 0){
					goto jDone;
				}
				unsigned char *const pbyWriteBegin = GetReservedData(&g_vStdOut);
				unsigned char *pbyWrite = pbyWriteBegin;
				memcpy(pbyWrite, pData, uSize);
				pbyWrite += uSize;
				Adopt(&g_vStdOut, (size_t)(pbyWrite - pbyWriteBegin));
				bSuccess = true;
			}
		}
	jDone:
		if(IsBuffered(&g_vStdOut)){
			Flush(&g_vStdOut, STDOUT_FLUSH_THRESHOLD);
		} else {
			Flush(&g_vStdOut, 0);
		}
		Unlock(&g_vStdOut);
	} else {
		dwErrorCode = ERROR_BROKEN_PIPE;
	}
	if(!bSuccess){
		SetLastError(dwErrorCode);
	}
	return bSuccess;
}
bool _MCFCRT_WriteStandardOutputChar32(char32_t c32CodePoint){
	bool bSuccess = false;
	DWORD dwErrorCode = 0;
	if(GetHandle(&g_vStdOut)){
		Lock(&g_vStdOut);
		{
			if(IsConsole(&g_vStdOut)){
				// UTF-8 <= UTF-32: 1:1, 2:1, 3:1, 4:1 (max)
				dwErrorCode = ReserveMore(&g_vStdOut, 2 * sizeof(char16_t));
				if(dwErrorCode != 0){
					goto jDone;
				}
				char16_t *const pc16WriteBegin = GetReservedData(&g_vStdOut);
				char16_t *pc16Write = pc16WriteBegin;
				_MCFCRT_EncodeUtf16(&pc16Write, c32CodePoint);
				Adopt(&g_vStdOut, (size_t)((char *)pc16Write - (char *)pc16WriteBegin));
				bSuccess = true;
			} else {
				// UTF-16 <= UTF-32: 1:1, 2:1 (max)
				dwErrorCode = ReserveMore(&g_vStdOut, 4);
				if(dwErrorCode != 0){
					goto jDone;
				}
				char *const pchWriteBegin = GetReservedData(&g_vStdOut);
				char *pchWrite = pchWriteBegin;
				_MCFCRT_EncodeUtf8(&pchWrite, c32CodePoint);
				Adopt(&g_vStdOut, (size_t)(pchWrite - pchWriteBegin));
				bSuccess = true;
			}
		}
	jDone:
		if(IsBuffered(&g_vStdOut)){
			Flush(&g_vStdOut, STDOUT_FLUSH_THRESHOLD);
		} else {
			Flush(&g_vStdOut, 0);
		}
		Unlock(&g_vStdOut);
	} else {
		dwErrorCode = ERROR_BROKEN_PIPE;
	}
	if(!bSuccess){
		SetLastError(dwErrorCode);
	}
	return bSuccess;
}
bool _MCFCRT_WriteStandardOutputText(const wchar_t *restrict pwcText, size_t uLength, bool bAppendNewLine){
	bool bSuccess = false;
	DWORD dwErrorCode = 0;
	if(GetHandle(&g_vStdOut)){
		Lock(&g_vStdOut);
		{
			if(IsConsole(&g_vStdOut)){
				dwErrorCode = ReserveMore(&g_vStdOut, (uLength + 1) * sizeof(char16_t));
				if(dwErrorCode != 0){
					goto jDone;
				}
				char16_t *const pc16WriteBegin = GetReservedData(&g_vStdOut);
				char16_t *pc16Write = pc16WriteBegin;
				memcpy(pc16Write, pwcText, uLength * sizeof(char16_t));
				pc16Write += uLength;
				if(bAppendNewLine){
					*(pc16Write++) = u'\n';
				}
				Adopt(&g_vStdOut, (size_t)((char *)pc16Write - (char *)pc16WriteBegin));
				bSuccess = true;
			} else {
				// UTF-8 <= UTF-16: 1:1, 2:1, 3:1 (max), 4:2
				if(uLength > (SIZE_MAX - 1) / sizeof(char16_t) / 3){
					dwErrorCode = ERROR_NOT_ENOUGH_MEMORY;
					goto jDone;
				}
				dwErrorCode = ReserveMore(&g_vStdOut, uLength * 3 * sizeof(char16_t) + 1);
				if(dwErrorCode != 0){
					goto jDone;
				}
				char *const pchWriteBegin = GetReservedData(&g_vStdOut);
				char *pchWrite = pchWriteBegin;
				const char16_t *pc16Read = (const void *)pwcText;
				const char16_t *const pc16ReadEnd = pc16Read + uLength;
				while(pc16Read < pc16ReadEnd){
					_MCFCRT_EncodeUtf8FromUtf16(&pchWrite, &pc16Read);
				}
				if(bAppendNewLine){
					*(pchWrite++) = '\n';
				}
				Adopt(&g_vStdOut, (size_t)(pchWrite - pchWriteBegin));
				bSuccess = true;
			}
		}
	jDone:
		if(IsBuffered(&g_vStdOut)){
			Flush(&g_vStdOut, STDOUT_FLUSH_THRESHOLD);
		} else {
			Flush(&g_vStdOut, 0);
		}
		Unlock(&g_vStdOut);
	} else {
		dwErrorCode = ERROR_BROKEN_PIPE;
	}
	if(!bSuccess){
		SetLastError(dwErrorCode);
	}
	return bSuccess;
}
bool _MCFCRT_IsStandardOutputBuffered(void){
	bool bBuffered;
	if(GetHandle(&g_vStdOut)){
		Lock(&g_vStdOut);
		{
			bBuffered = IsBuffered(&g_vStdOut);
		}
		Unlock(&g_vStdOut);
	} else {
		bBuffered = false;
	}
	return bBuffered;
}
void _MCFCRT_SetStandardOutputBuffered(bool bBuffered){
	if(GetHandle(&g_vStdOut)){
		Lock(&g_vStdOut);
		{
			if(!bBuffered){
				Flush(&g_vStdOut, 0); // 忽略错误。
			}
			SetBuffered(&g_vStdOut, bBuffered);
		}
		Unlock(&g_vStdOut);
	} else {
		// dwErrorCode = ERROR_BROKEN_PIPE;
	}
}
bool _MCFCRT_FlushStandardOutput(bool bHard){
	bool bSuccess = false;
	DWORD dwErrorCode = 0;
	if(GetHandle(&g_vStdOut)){
		Lock(&g_vStdOut);
		{
			dwErrorCode = Flush(&g_vStdOut, 0);
			if(dwErrorCode != 0){
				goto jDone;
			}
			if(bHard){
				dwErrorCode = FlushSystemBuffers(&g_vStdOut);
				if(dwErrorCode != 0){
					goto jDone;
				}
			}
			bSuccess = true;
		}
	jDone:
		Unlock(&g_vStdOut);
	} else {
		dwErrorCode = ERROR_BROKEN_PIPE;
	}
	if(!bSuccess){
		SetLastError(dwErrorCode);
	}
	return bSuccess;
}

bool _MCFCRT_WriteStandardErrorByte(unsigned char byData){
	_MCFCRT_FlushStandardOutput(false);

	bool bSuccess = false;
	DWORD dwErrorCode = 0;
	if(GetHandle(&g_vStdErr)){
		Lock(&g_vStdErr);
		{
			if(IsConsole(&g_vStdErr)){
				dwErrorCode = ReserveMore(&g_vStdErr, sizeof(char16_t));
				if(dwErrorCode != 0){
					goto jDone;
				}
				char16_t *const pc16WriteBegin = GetReservedData(&g_vStdErr);
				char16_t *pc16Write = pc16WriteBegin;
				*(pc16Write++) = (char16_t)(byData & 0x00FF);
				Adopt(&g_vStdErr, (size_t)((char *)pc16Write - (char *)pc16WriteBegin));
				bSuccess = true;
			} else {
				dwErrorCode = ReserveMore(&g_vStdErr, 1);
				if(dwErrorCode != 0){
					goto jDone;
				}
				unsigned char *const pbyWriteBegin = GetReservedData(&g_vStdErr);
				unsigned char *pbyWrite = pbyWriteBegin;
				*(pbyWrite++) = byData;
				Adopt(&g_vStdErr, (size_t)(pbyWrite - pbyWriteBegin));
				bSuccess = true;
			}
		}
	jDone:
		{
			Flush(&g_vStdErr, 0);
		}
		Unlock(&g_vStdErr);
	} else {
		dwErrorCode = ERROR_BROKEN_PIPE;
	}
	if(!bSuccess){
		SetLastError(dwErrorCode);
	}
	return bSuccess;
}
bool _MCFCRT_WriteStandardErrorBinary(const void *restrict pData, size_t uSize){
	_MCFCRT_FlushStandardOutput(false);

	bool bSuccess = false;
	DWORD dwErrorCode = 0;
	if(GetHandle(&g_vStdErr)){
		Lock(&g_vStdErr);
		{
			if(IsConsole(&g_vStdErr)){
				// UTF-16 <= UTF-8: 1:1 (max), 1:2, 1:3, 2:4
				if(uSize > SIZE_MAX / sizeof(char16_t)){
					dwErrorCode = ERROR_NOT_ENOUGH_MEMORY;
					goto jDone;
				}
				dwErrorCode = ReserveMore(&g_vStdErr, uSize * sizeof(char16_t));
				if(dwErrorCode != 0){
					goto jDone;
				}
				char16_t *const pc16WriteBegin = GetReservedData(&g_vStdErr);
				char16_t *pc16Write = pc16WriteBegin;
				const char *pchRead = pData;
				const char *const pchReadEnd = pchRead + uSize;
				while(pchRead < pchReadEnd){
					_MCFCRT_EncodeUtf16FromUtf8(&pc16Write, &pchRead);
				}
				Adopt(&g_vStdErr, (size_t)((char *)pc16Write - (char *)pc16WriteBegin));
				bSuccess = true;
			} else {
				dwErrorCode = ReserveMore(&g_vStdErr, uSize);
				if(dwErrorCode != 0){
					goto jDone;
				}
				unsigned char *const pbyWriteBegin = GetReservedData(&g_vStdErr);
				unsigned char *pbyWrite = pbyWriteBegin;
				memcpy(pbyWrite, pData, uSize);
				pbyWrite += uSize;
				Adopt(&g_vStdErr, (size_t)(pbyWrite - pbyWriteBegin));
				bSuccess = true;
			}
		}
	jDone:
		{
			Flush(&g_vStdErr, 0);
		}
		Unlock(&g_vStdErr);
	} else {
		dwErrorCode = ERROR_BROKEN_PIPE;
	}
	if(!bSuccess){
		SetLastError(dwErrorCode);
	}
	return bSuccess;
}
bool _MCFCRT_WriteStandardErrorChar32(char32_t c32CodePoint){
	_MCFCRT_FlushStandardOutput(false);

	bool bSuccess = false;
	DWORD dwErrorCode = 0;
	if(GetHandle(&g_vStdErr)){
		Lock(&g_vStdErr);
		{
			if(IsConsole(&g_vStdErr)){
				// UTF-16 <= UTF-32: 1:1, 2:1 (max)
				dwErrorCode = ReserveMore(&g_vStdErr, 2 * sizeof(char16_t));
				if(dwErrorCode != 0){
					goto jDone;
				}
				char16_t *const pc16WriteBegin = GetReservedData(&g_vStdErr);
				char16_t *pc16Write = pc16WriteBegin;
				_MCFCRT_EncodeUtf16(&pc16Write, c32CodePoint);
				Adopt(&g_vStdErr, (size_t)((char *)pc16Write - (char *)pc16WriteBegin));
				bSuccess = true;
			} else {
				// UTF-8 <= UTF-32: 1:1, 2:1, 3:1, 4:1 (max)
				dwErrorCode = ReserveMore(&g_vStdErr, 4);
				if(dwErrorCode != 0){
					goto jDone;
				}
				char *const pchWriteBegin = GetReservedData(&g_vStdErr);
				char *pchWrite = pchWriteBegin;
				_MCFCRT_EncodeUtf8(&pchWrite, c32CodePoint);
				Adopt(&g_vStdErr, (size_t)(pchWrite - pchWriteBegin));
				bSuccess = true;
			}
		}
	jDone:
		{
			Flush(&g_vStdErr, 0);
		}
		Unlock(&g_vStdErr);
	} else {
		dwErrorCode = ERROR_BROKEN_PIPE;
	}
	if(!bSuccess){
		SetLastError(dwErrorCode);
	}
	return bSuccess;
}
bool _MCFCRT_WriteStandardErrorText(const wchar_t *restrict pwcText, size_t uLength, bool bAppendNewLine){
	_MCFCRT_FlushStandardOutput(false);

	bool bSuccess = false;
	DWORD dwErrorCode = 0;
	if(GetHandle(&g_vStdErr)){
		Lock(&g_vStdErr);
		{
			if(IsConsole(&g_vStdErr)){
				dwErrorCode = ReserveMore(&g_vStdErr, (uLength + 1) * sizeof(char16_t));
				if(dwErrorCode != 0){
					goto jDone;
				}
				char16_t *const pc16WriteBegin = GetReservedData(&g_vStdErr);
				char16_t *pc16Write = pc16WriteBegin;
				memcpy(pc16Write, pwcText, uLength * sizeof(char16_t));
				pc16Write += uLength;
				if(bAppendNewLine){
					*(pc16Write++) = u'\n';
				}
				Adopt(&g_vStdErr, (size_t)((char *)pc16Write - (char *)pc16WriteBegin));
				bSuccess = true;
			} else {
				// UTF-8 <= UTF-16: 1:1, 2:1, 3:1 (max), 4:2
				if(uLength > (SIZE_MAX - 1) / sizeof(char16_t) / 3){
					dwErrorCode = ERROR_NOT_ENOUGH_MEMORY;
					goto jDone;
				}
				dwErrorCode = ReserveMore(&g_vStdErr, uLength * 3 * sizeof(char16_t) + 1);
				if(dwErrorCode != 0){
					goto jDone;
				}
				char *const pchWriteBegin = GetReservedData(&g_vStdErr);
				char *pchWrite = pchWriteBegin;
				const char16_t *pc16Read = (const void *)pwcText;
				const char16_t *const pc16ReadEnd = pc16Read + uLength;
				while(pc16Read < pc16ReadEnd){
					_MCFCRT_EncodeUtf8FromUtf16(&pchWrite, &pc16Read);
				}
				if(bAppendNewLine){
					*(pchWrite++) = '\n';
				}
				Adopt(&g_vStdErr, (size_t)(pchWrite - pchWriteBegin));
				bSuccess = true;
			}
		}
	jDone:
		{
			Flush(&g_vStdErr, 0);
		}
		Unlock(&g_vStdErr);
	} else {
		dwErrorCode = ERROR_BROKEN_PIPE;
	}
	if(!bSuccess){
		SetLastError(dwErrorCode);
	}
	return bSuccess;
}
bool _MCFCRT_FlushStandardError(bool bHard){
	bool bSuccess = false;
	DWORD dwErrorCode = 0;
	if(GetHandle(&g_vStdErr)){
		Lock(&g_vStdErr);
		{
			dwErrorCode = Flush(&g_vStdErr, 0);
			if(dwErrorCode != 0){
				goto jDone;
			}
			if(bHard){
				dwErrorCode = FlushSystemBuffers(&g_vStdErr);
				if(dwErrorCode != 0){
					goto jDone;
				}
			}
			bSuccess = true;
		}
	jDone:
		Unlock(&g_vStdErr);
	} else {
		dwErrorCode = ERROR_BROKEN_PIPE;
	}
	if(!bSuccess){
		SetLastError(dwErrorCode);
	}
	return bSuccess;
}
