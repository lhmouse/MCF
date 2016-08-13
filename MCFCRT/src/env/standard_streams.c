// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "standard_streams.h"
#include "../ext/utf.h"
#include "mutex.h"
#include "mcfwin.h"

#define STDIN_POPULATION_SIZE    0x1000u
#define STDOUT_FLUSH_THRESHOLD   0x1000u

// _MCFCRT_ASSERT() 会操作标准输入输出流，所以可能会死锁。
#ifdef NDEBUG
#	define SS_ASSERT(expr_)   ((void)((expr_) || (__builtin_unreachable(), 1)))
#else
#	define SS_ASSERT(expr_)   ((void)((expr_) || (__builtin_trap(), 1)))
#endif

typedef struct tagStream {
	_MCFCRT_Mutex vMutex;
	HANDLE hPipe;
	bool bConsole;
	bool bEchoing;
	bool bBuffered;

	void *pStorageBegin;
	void *pDataBegin;
	void *pDataEnd;
	void *pStorageEnd;
} Stream;

static inline void Reset(Stream *restrict pStream, HANDLE hPipe, bool bBuffered){
	char *const pchStorageBegin = pStream->pStorageBegin;

	if(pchStorageBegin){
		HeapFree(GetProcessHeap(), 0, pchStorageBegin);
	}

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
	pStream->pStorageBegin = nullptr;
	pStream->pDataBegin    = nullptr;
	pStream->pDataEnd      = nullptr;
	pStream->pStorageEnd   = nullptr;
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
			const size_t uSafetyBumperSize = 4;
			if(uNewCapacity > SIZE_MAX - uSafetyBumperSize){
				return ERROR_NOT_ENOUGH_MEMORY;
			}
			char *const pchNewStorage = HeapAlloc(GetProcessHeap(), 0, uNewCapacity + uSafetyBumperSize);
			if(!pchNewStorage){
				return ERROR_NOT_ENOUGH_MEMORY;
			}
			if(pchStorageBegin){
				memcpy(pchNewStorage, pchDataBegin, uOldSize);
				HeapFree(GetProcessHeap(), 0, pchStorageBegin);
			}
			memset(pchNewStorage + uNewCapacity, 0, uSafetyBumperSize);

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

static inline DWORD Populate(Stream *restrict pStream, size_t uThreshold){
	return 0;
}
static inline DWORD Flush(Stream *restrict pStream, size_t uThreshold){
	DWORD dwErrorCode = 0;
	for(;;){
		size_t uBytesToWrite = GetSize(pStream);
		if(uBytesToWrite < uThreshold){
			break;
		}
		if(uBytesToWrite > UINT32_MAX){
			uBytesToWrite = UINT32_MAX;
		}
		size_t uBytesWritten;
		if(IsConsole(pStream)){
			uBytesWritten = uBytesToWrite;
/*
			wchar_t awcTemp[512];
			DWORD dwCharsToWrite = 0;
			DWORD dwBytesWritten;
			const char *const pchReadBegin = GetData(pStream);
			for(;;){
				char16_t *pc16Write = (void *)(awcTemp + dwCharsToWrite);
				const char *pchRead = pchReadBegin +
				_MCFCRT_EncodeUtf16FromUtf8(&pchRead,
			}


			const DWORD dwCharsToWrite = (DWORD)(uBytesToWrite / sizeof(wchar_t));
			if(dwCharsToWrite == 0){
				break;
			}
			DWORD dwCharsWritten;
			if(!WriteConsoleW(GetHandle(pStream), GetData(pStream), dwCharsToWrite, &dwCharsWritten, nullptr)){
				dwErrorCode = GetLastError();
				break;
			}
			uBytesWritten = dwCharsWritten * sizeof(wchar_t);
*/
		} else {
			DWORD dwBytesToWrite = (DWORD)uBytesToWrite;
			if(dwBytesToWrite == 0){
				break;
			}
			DWORD dwBytesWritten;
			if(!WriteFile(GetHandle(pStream), GetData(pStream), dwBytesToWrite, &dwBytesWritten, nullptr)){
				dwErrorCode = GetLastError();
			}
			uBytesWritten = dwBytesWritten;
		}
		Discard(pStream, uBytesWritten);
		if(dwErrorCode != 0){
			break;
		}
	}
	return dwErrorCode;
}
static inline DWORD FlushSystemBuffers(Stream *restrict pStream){
	return 0;
}

static inline bool IsEchoing(const Stream *restrict pStream){
	return pStream->bEchoing;
}
static inline bool SetEchoing(Stream *restrict pStream, bool bEchoing){
	return false;
}

static inline bool IsBuffered(const Stream *restrict pStream){
	return pStream->bBuffered;
}
static inline bool SetBuffered(Stream *restrict pStream, bool bBuffered){
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
	int nRet = -1;
	DWORD dwErrorCode = 0;
	if(GetHandle(&g_vStdIn)){
		Lock(&g_vStdIn);
		{
			size_t uSizeAvail = GetSize(&g_vStdIn);
			if(uSizeAvail == 0){
				dwErrorCode = Populate(&g_vStdIn, STDIN_POPULATION_SIZE);
				if(dwErrorCode != 0){
					goto jDone;
				}
				uSizeAvail = GetSize(&g_vStdIn);
				if(uSizeAvail == 0){
					dwErrorCode = ERROR_HANDLE_EOF;
					goto jDone;
				}
			}
			unsigned char byData;
			memcpy(&byData, GetData(&g_vStdIn), 1);
			nRet = byData;
		}
	jDone:
		Unlock(&g_vStdIn);
	} else {
		dwErrorCode = ERROR_BROKEN_PIPE;
	}
	if(dwErrorCode != 0){
		SetLastError(dwErrorCode);
		return -1;
	}
	return nRet;
}
int _MCFCRT_ReadStandardInputByte(void){
	int nRet = -1;
	DWORD dwErrorCode = 0;
	if(GetHandle(&g_vStdIn)){
		Lock(&g_vStdIn);
		{
			size_t uSizeAvail = GetSize(&g_vStdIn);
			if(uSizeAvail == 0){
				dwErrorCode = Populate(&g_vStdIn, STDIN_POPULATION_SIZE);
				if(dwErrorCode != 0){
					goto jDone;
				}
				uSizeAvail = GetSize(&g_vStdIn);
				if(uSizeAvail == 0){
					dwErrorCode = ERROR_HANDLE_EOF;
					goto jDone;
				}
			}
			unsigned char byData;
			memcpy(&byData, GetData(&g_vStdIn), 1);
			Discard(&g_vStdIn, 1);
			nRet = byData;
		}
	jDone:
		Unlock(&g_vStdIn);
	} else {
		dwErrorCode = ERROR_BROKEN_PIPE;
	}
	if(dwErrorCode != 0){
		SetLastError(dwErrorCode);
		return -1;
	}
	return nRet;
}
size_t _MCFCRT_PeekStandardInputBinary(void *restrict pData, size_t uSize){
	size_t uBytesRead = 0;
	DWORD dwErrorCode = 0;
	if(GetHandle(&g_vStdIn)){
		Lock(&g_vStdIn);
		{
			size_t uSizeAvail = GetSize(&g_vStdIn);
			if(uSizeAvail == 0){
				dwErrorCode = Populate(&g_vStdIn, STDIN_POPULATION_SIZE);
				if(dwErrorCode != 0){
					goto jDone;
				}
				uSizeAvail = GetSize(&g_vStdIn);
				if(uSizeAvail == 0){
					dwErrorCode = ERROR_HANDLE_EOF;
					goto jDone;
				}
			}
			if(uSizeAvail > uSize){
				uSizeAvail = uSize;
			}
			memcpy(pData, GetData(&g_vStdIn), uSizeAvail);
			uBytesRead = uSizeAvail;
		}
	jDone:
		Unlock(&g_vStdIn);
	} else {
		dwErrorCode = ERROR_BROKEN_PIPE;
	}
	if(dwErrorCode != 0){
		SetLastError(dwErrorCode);
		return 0;
	}
	return uBytesRead;
}
size_t _MCFCRT_ReadStandardInputBinary(void *restrict pData, size_t uSize){
	size_t uBytesRead = 0;
	DWORD dwErrorCode = 0;
	if(GetHandle(&g_vStdIn)){
		Lock(&g_vStdIn);
		{
			size_t uSizeAvail = GetSize(&g_vStdIn);
			if(uSizeAvail == 0){
				dwErrorCode = Populate(&g_vStdIn, STDIN_POPULATION_SIZE);
				if(dwErrorCode != 0){
					goto jDone;
				}
				uSizeAvail = GetSize(&g_vStdIn);
				if(uSizeAvail == 0){
					dwErrorCode = ERROR_HANDLE_EOF;
					goto jDone;
				}
			}
			if(uSizeAvail > uSize){
				uSizeAvail = uSize;
			}
			memcpy(pData, GetData(&g_vStdIn), uSizeAvail);
			Discard(&g_vStdIn, uSizeAvail);
			uBytesRead = uSizeAvail;
		}
	jDone:
		Unlock(&g_vStdIn);
	} else {
		dwErrorCode = ERROR_BROKEN_PIPE;
	}
	if(dwErrorCode != 0){
		SetLastError(dwErrorCode);
		return 0;
	}
	return uBytesRead;
}
size_t _MCFCRT_DiscardStandardInputBinary(size_t uSize){
	size_t uBytesDiscarded = 0;
	DWORD dwErrorCode = 0;
	if(GetHandle(&g_vStdIn)){
		Lock(&g_vStdIn);
		{
			size_t uSizeAvail = GetSize(&g_vStdIn);
			if(uSizeAvail == 0){
				dwErrorCode = Populate(&g_vStdIn, STDIN_POPULATION_SIZE);
				if(dwErrorCode != 0){
					goto jDone;
				}
				uSizeAvail = GetSize(&g_vStdIn);
				if(uSizeAvail == 0){
					dwErrorCode = ERROR_HANDLE_EOF;
					goto jDone;
				}
			}
			if(uSizeAvail > uSize){
				uSizeAvail = uSize;
			}
			Discard(&g_vStdIn, uSizeAvail);
			uBytesDiscarded = uSizeAvail;
		}
	jDone:
		Unlock(&g_vStdIn);
	} else {
		dwErrorCode = ERROR_BROKEN_PIPE;
	}
	if(dwErrorCode != 0){
		SetLastError(dwErrorCode);
		return 0;
	}
	return uBytesDiscarded;
}
/*long _MCFCRT_PeekStandardInputChar32(void){
}
long _MCFCRT_ReadStandardInputChar32(void){
}
size_t _MCFCRT_PeekStandardInputText(wchar_t *restrict pwcText, size_t uLength, bool bSingleLine){
}
size_t _MCFCRT_ReadStandardInputText(wchar_t *restrict pwcText, size_t uLength, bool bSingleLine){
}
size_t _MCFCRT_DiscardStandardInputText(size_t uLength, bool bSingleLine){
}*/
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
	DWORD dwErrorCode = 0;
	if(GetHandle(&g_vStdOut)){
		Lock(&g_vStdIn);
		{
			dwErrorCode = SetEchoing(&g_vStdIn, bEchoing);
		}
		Unlock(&g_vStdIn);
	} else {
		dwErrorCode = ERROR_BROKEN_PIPE;
	}
	if(dwErrorCode != 0){
		SetLastError(dwErrorCode);
		return false;
	}
	return true;
}

bool _MCFCRT_WriteStandardOutputByte(unsigned char byData){
	DWORD dwErrorCode = 0;
	if(GetHandle(&g_vStdOut)){
		Lock(&g_vStdOut);
		{
			dwErrorCode = ReserveMore(&g_vStdOut, 1);
			if(dwErrorCode != 0){
				goto jDone;
			}
			char *const pchWriteBegin = GetReservedData(&g_vStdOut);
			memcpy(pchWriteBegin, &byData, 1);
			Adopt(&g_vStdOut, 1);
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
	if(dwErrorCode != 0){
		SetLastError(dwErrorCode);
		return false;
	}
	return true;
}
bool _MCFCRT_WriteStandardOutputBinary(const void *restrict pData, size_t uSize){
	DWORD dwErrorCode = 0;
	if(GetHandle(&g_vStdOut)){
		Lock(&g_vStdOut);
		{
			dwErrorCode = ReserveMore(&g_vStdOut, uSize);
			if(dwErrorCode != 0){
				goto jDone;
			}
			char *const pchWriteBegin = GetReservedData(&g_vStdOut);
			memcpy(pchWriteBegin, pData, uSize);
			Adopt(&g_vStdOut, uSize);
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
	if(dwErrorCode != 0){
		SetLastError(dwErrorCode);
		return false;
	}
	return true;
}
bool _MCFCRT_WriteStandardOutputChar32(char32_t c32CodePoint){
	DWORD dwErrorCode = 0;
	if(GetHandle(&g_vStdOut)){
		Lock(&g_vStdOut);
		{
			dwErrorCode = ReserveMore(&g_vStdOut, 4);
			if(dwErrorCode != 0){
				goto jDone;
			}
			char *const pchWriteBegin = GetReservedData(&g_vStdOut);
			char *pchWriteEnd = pchWriteBegin;
			_MCFCRT_EncodeUtf8(&pchWriteEnd, c32CodePoint);
			Adopt(&g_vStdOut, (size_t)(pchWriteEnd - pchWriteBegin));
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
	if(dwErrorCode != 0){
		SetLastError(dwErrorCode);
		return false;
	}
	return true;
}
bool _MCFCRT_WriteStandardOutputText(const wchar_t *restrict pwcText, size_t uLength, bool bAppendNewLine){
	DWORD dwErrorCode = 0;
	if(GetHandle(&g_vStdOut)){
		Lock(&g_vStdOut);
		{
			if(uLength > (SIZE_MAX - 1) / 3){
				dwErrorCode = ERROR_NOT_ENOUGH_MEMORY;
				goto jDone;
			}
			dwErrorCode = ReserveMore(&g_vStdOut, uLength * 3 + 1);
			if(dwErrorCode != 0){
				goto jDone;
			}
			char *const pchWriteBegin = GetReservedData(&g_vStdOut);
			char *pchWriteEnd = pchWriteBegin;
			const char16_t *pc16ReadBegin = (const void *)pwcText;
			const char16_t *const pc16ReadEnd = pc16ReadBegin + uLength;
			while(pc16ReadBegin < pc16ReadEnd){
				_MCFCRT_EncodeUtf8FromUtf16(&pchWriteEnd, &pc16ReadBegin);
			}
			if(bAppendNewLine){
				*pchWriteEnd = '\n';
				++pchWriteEnd;
			}
			Adopt(&g_vStdOut, (size_t)(pchWriteEnd - pchWriteBegin));
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
	if(dwErrorCode != 0){
		SetLastError(dwErrorCode);
		return false;
	}
	return true;
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
bool _MCFCRT_SetStandardOutputBuffered(bool bBuffered){
	DWORD dwErrorCode = 0;
	if(GetHandle(&g_vStdOut)){
		Lock(&g_vStdOut);
		{
			dwErrorCode = SetBuffered(&g_vStdOut, bBuffered);
		}
		Unlock(&g_vStdOut);
	} else {
		dwErrorCode = ERROR_BROKEN_PIPE;
	}
	if(dwErrorCode != 0){
		SetLastError(dwErrorCode);
		return false;
	}
	return true;
}
bool _MCFCRT_FlushStandardOutput(bool bHard){
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
		}
	jDone:
		Unlock(&g_vStdOut);
	} else {
		dwErrorCode = ERROR_BROKEN_PIPE;
	}
	if(dwErrorCode != 0){
		SetLastError(dwErrorCode);
		return false;
	}
	return true;
}

bool _MCFCRT_WriteStandardErrorByte(unsigned char byData){
	_MCFCRT_FlushStandardOutput(false);

	DWORD dwErrorCode = 0;
	if(GetHandle(&g_vStdErr)){
		Lock(&g_vStdErr);
		{
			dwErrorCode = ReserveMore(&g_vStdErr, 1);
			if(dwErrorCode != 0){
				goto jDone;
			}
			char *const pchWriteBegin = GetReservedData(&g_vStdErr);
			memcpy(pchWriteBegin, &byData, 1);
			Adopt(&g_vStdErr, 1);
		}
	jDone:
		{
			Flush(&g_vStdErr, 0);
		}
		Unlock(&g_vStdErr);
	} else {
		dwErrorCode = ERROR_BROKEN_PIPE;
	}
	if(dwErrorCode != 0){
		SetLastError(dwErrorCode);
		return false;
	}
	return true;
}
bool _MCFCRT_WriteStandardErrorBinary(const void *restrict pData, size_t uSize){
	_MCFCRT_FlushStandardOutput(false);

	DWORD dwErrorCode = 0;
	if(GetHandle(&g_vStdErr)){
		Lock(&g_vStdErr);
		{
			dwErrorCode = ReserveMore(&g_vStdErr, uSize);
			if(dwErrorCode != 0){
				goto jDone;
			}
			char *const pchWriteBegin = GetReservedData(&g_vStdErr);
			memcpy(pchWriteBegin, pData, uSize);
			Adopt(&g_vStdErr, uSize);
		}
	jDone:
		{
			Flush(&g_vStdErr, 0);
		}
		Unlock(&g_vStdErr);
	} else {
		dwErrorCode = ERROR_BROKEN_PIPE;
	}
	if(dwErrorCode != 0){
		SetLastError(dwErrorCode);
		return false;
	}
	return true;
}
bool _MCFCRT_WriteStandardErrorChar32(char32_t c32CodePoint){
	_MCFCRT_FlushStandardOutput(false);

	DWORD dwErrorCode = 0;
	if(GetHandle(&g_vStdErr)){
		Lock(&g_vStdErr);
		{
			dwErrorCode = ReserveMore(&g_vStdErr, 4);
			if(dwErrorCode != 0){
				goto jDone;
			}
			char *const pchWriteBegin = GetReservedData(&g_vStdErr);
			char *pchWriteEnd = pchWriteBegin;
			_MCFCRT_EncodeUtf8(&pchWriteEnd, c32CodePoint);
			Adopt(&g_vStdErr, (size_t)(pchWriteEnd - pchWriteBegin));
		}
	jDone:
		{
			Flush(&g_vStdErr, 0);
		}
		Unlock(&g_vStdErr);
	} else {
		dwErrorCode = ERROR_BROKEN_PIPE;
	}
	if(dwErrorCode != 0){
		SetLastError(dwErrorCode);
		return false;
	}
	return true;
}
bool _MCFCRT_WriteStandardErrorText(const wchar_t *restrict pwcText, size_t uLength, bool bAppendNewLine){
	_MCFCRT_FlushStandardOutput(false);

	DWORD dwErrorCode = 0;
	if(GetHandle(&g_vStdErr)){
		Lock(&g_vStdErr);
		{
			if(uLength > (SIZE_MAX - 1) / 3){
				dwErrorCode = ERROR_NOT_ENOUGH_MEMORY;
				goto jDone;
			}
			dwErrorCode = ReserveMore(&g_vStdErr, uLength * 3 + 1);
			if(dwErrorCode != 0){
				goto jDone;
			}
			char *const pchWriteBegin = GetReservedData(&g_vStdErr);
			char *pchWriteEnd = pchWriteBegin;
			const char16_t *pc16ReadBegin = (const void *)pwcText;
			const char16_t *const pc16ReadEnd = pc16ReadBegin + uLength;
			while(pc16ReadBegin < pc16ReadEnd){
				_MCFCRT_EncodeUtf8FromUtf16(&pchWriteEnd, &pc16ReadBegin);
			}
			if(bAppendNewLine){
				*pchWriteEnd = '\n';
				++pchWriteEnd;
			}
			Adopt(&g_vStdErr, (size_t)(pchWriteEnd - pchWriteBegin));
		}
	jDone:
		{
			Flush(&g_vStdErr, 0);
		}
		Unlock(&g_vStdErr);
	} else {
		dwErrorCode = ERROR_BROKEN_PIPE;
	}
	if(dwErrorCode != 0){
		SetLastError(dwErrorCode);
		return false;
	}
	return true;
}
bool _MCFCRT_FlushStandardError(bool bHard){
	_MCFCRT_FlushStandardOutput(false);

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
		}
	jDone:
		Unlock(&g_vStdErr);
	} else {
		dwErrorCode = ERROR_BROKEN_PIPE;
	}
	if(dwErrorCode != 0){
		SetLastError(dwErrorCode);
		return false;
	}
	return true;
}
