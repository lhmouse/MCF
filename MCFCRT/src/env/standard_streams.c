// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "standard_streams.h"
#include "../ext/utf.h"
#include "mutex.h"
#include "mcfwin.h"

static_assert(sizeof (wchar_t) == sizeof (char16_t), "What?");
static_assert(alignof(wchar_t) == alignof(char16_t), "What?");

static inline void *VirtualAllocHelper(size_t uSize){
	return VirtualAlloc(_MCFCRT_NULLPTR, uSize, MEM_COMMIT, PAGE_READWRITE);
}
static inline void VirtualFreeHelper(void *pAddress){
	if(!pAddress){
		return;
	}
	VirtualFree(pAddress, 0, MEM_RELEASE);
}

#define POPULATION_INCREMENT    8192ul
#define FLUSH_THRESHOLD         4096ul

typedef struct tagStream {
	_MCFCRT_Mutex vMutex;
	HANDLE hFile;
	bool bConsole;
	bool bEchoing;
	bool bBuffered;

	unsigned char *pbyBuffer;
	size_t uCapacity;
	size_t uTextBegin;
	size_t uTextEnd;
	size_t uBinaryBegin;
	size_t uBinaryEnd;
} Stream;

static void Lock(Stream *restrict pStream){
	_MCFCRT_WaitForMutexForever(&(pStream->vMutex), _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
}
static void Unlock(Stream *restrict pStream){
	_MCFCRT_SignalMutex(&(pStream->vMutex));
}
static void UnlockAndSetLastError(Stream *restrict pStream, DWORD dwErrorCode){
	Unlock(pStream);
	SetLastError(dwErrorCode);
}

static DWORD UnlockedSetConsoleMode(Stream *restrict pStream, DWORD dwModeToRemove, DWORD dwModeToAdd){
	if(pStream->bConsole){
		DWORD dwConsoleMode;
		if(!GetConsoleMode(pStream->hFile, &dwConsoleMode)){
			return GetLastError();
		}
		if(!SetConsoleMode(pStream->hFile, (dwConsoleMode & ~dwModeToRemove) | dwModeToAdd)){
			return GetLastError();
		}
		return 0;
	} else {
		return ERROR_INVALID_HANDLE;
	}
}
static DWORD UnlockedReserve(Stream *restrict pStream, size_t uTextSizeAdd, size_t uBinarySizeAdd){
	const size_t uTextSize = pStream->uTextEnd - pStream->uTextBegin;
	size_t uTextSizeToReserve;
	if(__builtin_add_overflow(uTextSize, uTextSizeAdd, &uTextSizeToReserve)){
		return ERROR_NOT_ENOUGH_MEMORY;
	}
	const size_t uBinarySize = pStream->uBinaryEnd - pStream->uBinaryBegin;
	size_t uBinarySizeToReserve;
	if(__builtin_add_overflow(uBinarySize, uBinarySizeAdd, &uBinarySizeToReserve)){
		return ERROR_NOT_ENOUGH_MEMORY;
	}
	if(((pStream->uBinaryBegin - pStream->uTextEnd) < uTextSizeAdd) || ((pStream->uCapacity - pStream->uBinaryEnd) < uBinarySizeAdd)){
		size_t uMinimumSizeToReserve;
		if(__builtin_add_overflow(uTextSizeToReserve, uBinarySizeToReserve, &uMinimumSizeToReserve)){
			return ERROR_NOT_ENOUGH_MEMORY;
		}
		unsigned char *pbyBufferOld = pStream->pbyBuffer;
		unsigned char *pbyBufferNew = pbyBufferOld;
		size_t uCapacityNew = pStream->uCapacity;
		if(uCapacityNew < uMinimumSizeToReserve){
			uCapacityNew += pStream->uCapacity / 2;
			uCapacityNew |= uMinimumSizeToReserve;
			// Round the size up to the next 64KiB boundary.
			// See <https://blogs.msdn.microsoft.com/oldnewthing/20031008-00/?p=42223>.
			uCapacityNew += 0xFFFF;
			uCapacityNew &= (size_t)-0x10000;
			if(uCapacityNew < uMinimumSizeToReserve){
				return ERROR_NOT_ENOUGH_MEMORY;
			}
			pbyBufferNew = VirtualAllocHelper(uCapacityNew);
			if(!pbyBufferNew){
				return GetLastError();
			}
		}
		if((uTextSize != 0) && (pbyBufferNew != pbyBufferOld + pStream->uTextBegin)){
			memmove(pbyBufferNew, pbyBufferOld + pStream->uTextBegin, uTextSize);
		}
		if((uBinarySize != 0) && (pbyBufferNew + uTextSizeToReserve != pbyBufferOld + pStream->uBinaryBegin)){
			memmove(pbyBufferNew + uTextSizeToReserve, pbyBufferOld + pStream->uBinaryBegin, uBinarySize);
		}
		if(pbyBufferNew != pbyBufferOld){
			VirtualFreeHelper(pbyBufferOld);
			pStream->pbyBuffer = pbyBufferNew;
			pStream->uCapacity = uCapacityNew;
		}
		pStream->uTextBegin = 0;
		pStream->uTextEnd = uTextSize;
		pStream->uBinaryBegin = uTextSizeToReserve;
		pStream->uBinaryEnd = uTextSizeToReserve + uBinarySize;
	}
	return 0;
}
static DWORD UnlockedConvertBinaryToText(Stream *restrict pStream, bool bExhaust){
	const size_t uBytesAvail = pStream->uBinaryEnd - pStream->uBinaryBegin;
	if(uBytesAvail > 0){
		// Max(UTF-16:UTF-8) = Max(2:1, 2:2, 2:3, 4:4) = 2
		size_t uTextSizeAdd;
		if(__builtin_mul_overflow(uBytesAvail, 2, &uTextSizeAdd)){
			return ERROR_NOT_ENOUGH_MEMORY;
		}
		DWORD dwErrorCode = UnlockedReserve(pStream, uTextSizeAdd, 0);
		if(dwErrorCode != 0){
			return dwErrorCode;
		}
		const char *pchRead = (void *)(pStream->pbyBuffer + pStream->uBinaryBegin);
		const char *const pchReadEnd = (void *)(pStream->pbyBuffer + pStream->uBinaryEnd);
		wchar_t *pwcWrite = (void *)(pStream->pbyBuffer + pStream->uTextEnd);
		for(;;){
			char32_t c32CodePoint = _MCFCRT_DecodeUtf8(&pchRead, pchReadEnd, true);
			if(!_MCFCRT_UTF_SUCCESS(c32CodePoint)){
				break;
			}
			_MCFCRT_UncheckedEncodeUtf16(&pwcWrite, c32CodePoint, true);
		}
		if(bExhaust){
			while(pchRead != pchReadEnd){
				_MCFCRT_UncheckedEncodeUtf16(&pwcWrite, (uint8_t)*(pchRead++), true);
			}
		}
		pStream->uBinaryBegin = (size_t)((unsigned char *)pchRead - pStream->pbyBuffer);
		pStream->uTextEnd = (size_t)((unsigned char *)pwcWrite - pStream->pbyBuffer);
	}
	return 0;
}
static DWORD UnlockedConvertTextToBinary(Stream *restrict pStream, bool bExhaust){
	const size_t uBytesAvail = pStream->uTextEnd - pStream->uTextBegin;
	if(uBytesAvail > 0){
		// Max(UTF-8:UTF-16) = Max(1:2, 2:2, 3:2, 4:4) = 1.5
		size_t uBinarySizeAdd;
		if(__builtin_mul_overflow(uBytesAvail, 2, &uBinarySizeAdd)){
			return ERROR_NOT_ENOUGH_MEMORY;
		}
		DWORD dwErrorCode = UnlockedReserve(pStream, 0, uBinarySizeAdd);
		if(dwErrorCode != 0){
			return dwErrorCode;
		}
		const wchar_t *pwcRead = (void *)(pStream->pbyBuffer + pStream->uTextBegin);
		const wchar_t *const pwcReadEnd = (void *)(pStream->pbyBuffer + pStream->uTextEnd);
		char *pchWrite = (void *)(pStream->pbyBuffer + pStream->uBinaryEnd);
		for(;;){
			char32_t c32CodePoint = _MCFCRT_DecodeUtf16(&pwcRead, pwcReadEnd, true);
			if(!_MCFCRT_UTF_SUCCESS(c32CodePoint)){
				break;
			}
			_MCFCRT_UncheckedEncodeUtf8(&pchWrite, c32CodePoint, true);
		}
		if(bExhaust){
			while(pwcRead != pwcReadEnd){
				_MCFCRT_UncheckedEncodeUtf8(&pchWrite, (uint16_t)*(pwcRead++), true);
			}
		}
		pStream->uTextBegin = (size_t)((unsigned char *)pwcRead - pStream->pbyBuffer);
		pStream->uBinaryEnd = (size_t)((unsigned char *)pchWrite - pStream->pbyBuffer);
	}
	return 0;
}
static DWORD UnlockedPopulate(Stream *restrict pStream){
	if(pStream->bConsole){
		DWORD dwErrorCode = UnlockedReserve(pStream, POPULATION_INCREMENT, 0);
		if(dwErrorCode != 0){
			return dwErrorCode;
		}
		DWORD dwCharsRead;
		if(!ReadConsoleW(pStream->hFile, (void *)(pStream->pbyBuffer + pStream->uTextEnd), POPULATION_INCREMENT / sizeof(wchar_t), &dwCharsRead, _MCFCRT_NULLPTR)){
			return GetLastError();
		}
		if(dwCharsRead == 0){
			return ERROR_HANDLE_EOF;
		}
		pStream->uTextEnd += dwCharsRead * sizeof(wchar_t);
		return 0;
	} else {
		DWORD dwErrorCode = UnlockedReserve(pStream, 0, POPULATION_INCREMENT);
		if(dwErrorCode != 0){
			return dwErrorCode;
		}
		DWORD dwBytesRead;
		if(!ReadFile(pStream->hFile, (void *)(pStream->pbyBuffer + pStream->uBinaryEnd), POPULATION_INCREMENT, &dwBytesRead, _MCFCRT_NULLPTR)){
			return GetLastError();
		}
		if(dwBytesRead == 0){
			return ERROR_HANDLE_EOF;
		}
		pStream->uBinaryEnd += dwBytesRead;
		return 0;
	}
}
static DWORD UnlockedFlush(Stream *restrict pStream, bool bHard){
	if(pStream->bConsole){
		DWORD dwErrorCode = UnlockedConvertBinaryToText(pStream, false);
		if(dwErrorCode != 0){
			return dwErrorCode;
		}
		for(;;){
			size_t uBytesAvail = pStream->uTextEnd - pStream->uTextBegin;
			if(uBytesAvail == 0){
				break;
			}
			if(uBytesAvail > UINT16_MAX){
				uBytesAvail = UINT16_MAX;
			}
			DWORD dwBytesToWrite = (DWORD)uBytesAvail;
			DWORD dwCharsWritten;
			if(!WriteConsoleW(pStream->hFile, (void *)(pStream->pbyBuffer + pStream->uTextBegin), dwBytesToWrite / sizeof(wchar_t), &dwCharsWritten, _MCFCRT_NULLPTR)){
				return GetLastError();
			}
			pStream->uTextBegin += dwCharsWritten * sizeof(wchar_t);
		}
		// `bHard` is unused.
		return 0;
	} else {
		DWORD dwErrorCode = UnlockedConvertTextToBinary(pStream, false);
		if(dwErrorCode != 0){
			return dwErrorCode;
		}
		for(;;){
			size_t uBytesAvail = pStream->uBinaryEnd - pStream->uBinaryBegin;
			if(uBytesAvail == 0){
				break;
			}
			if(uBytesAvail > UINT32_MAX){
				uBytesAvail = UINT32_MAX;
			}
			DWORD dwBytesToWrite = (DWORD)uBytesAvail;
			DWORD dwBytesWritten;
			if(!WriteFile(pStream->hFile, (void *)(pStream->pbyBuffer + pStream->uBinaryBegin), dwBytesToWrite, &dwBytesWritten, _MCFCRT_NULLPTR)){
				return GetLastError();
			}
			pStream->uBinaryBegin += dwBytesWritten;
		}
		if(bHard){
			// Errors are ignored.
			FlushFileBuffers(pStream->hFile);
		}
		return 0;
	}
}
static void UnlockedReset(Stream *restrict pStream, HANDLE hFile, bool bBuffered){
	// Errors are ignored.
	UnlockedFlush(pStream, true);
	VirtualFreeHelper(pStream->pbyBuffer);
	pStream->pbyBuffer = _MCFCRT_NULLPTR;
	pStream->uCapacity = 0;
	pStream->uBinaryBegin = 0;
	pStream->uBinaryEnd = 0;
	pStream->uTextBegin = 0;
	pStream->uTextEnd = 0;

	pStream->hFile = _MCFCRT_NULLPTR;
	pStream->bConsole = false;
	pStream->bEchoing = false;
	// Normalize `INVALID_HANDLE_VALUE` to `NULL`.
	if((hFile != INVALID_HANDLE_VALUE) && (hFile != _MCFCRT_NULLPTR)){
		pStream->hFile = hFile;
		DWORD dwConsoleMode;
		if(GetConsoleMode(hFile, &dwConsoleMode)){
			pStream->bConsole = true;
			pStream->bEchoing = dwConsoleMode & ENABLE_ECHO_INPUT;
		}
	}
	pStream->bBuffered = bBuffered;
}

static Stream g_scStdStreams[3];

#define Si  (g_scStdStreams + 0)
#define So  (g_scStdStreams + 1)
#define Se  (g_scStdStreams + 2)

bool __MCFCRT_StandardStreamsInit(void){
	UnlockedReset(Se, GetStdHandle(STD_ERROR_HANDLE), false);
	UnlockedReset(So, GetStdHandle(STD_OUTPUT_HANDLE), true);
	UnlockedReset(Si, GetStdHandle(STD_INPUT_HANDLE), false);
	return true;
}
void __MCFCRT_StandardStreamsUninit(void){
	UnlockedReset(Si, _MCFCRT_NULLPTR, false);
	UnlockedReset(So, _MCFCRT_NULLPTR, false);
	UnlockedReset(Se, _MCFCRT_NULLPTR, false);
}

long _MCFCRT_ReadStandardInputChar32(bool bDontRemove){
	_MCFCRT_FlushStandardOutput(false);

	if(!(Si->hFile)){
		SetLastError(ERROR_INVALID_HANDLE);
		return -1;
	}

	Lock(Si);
	const wchar_t *pwcRead;
	char32_t c32CodePoint;
	for(;;){
		DWORD dwErrorCode = UnlockedConvertBinaryToText(Si, false);
		if(dwErrorCode != 0){
			UnlockAndSetLastError(Si, dwErrorCode);
			return -1;
		}
		pwcRead = (void *)(Si->pbyBuffer + Si->uTextBegin);
		c32CodePoint = _MCFCRT_DecodeUtf16(&pwcRead, (void *)(Si->pbyBuffer + Si->uTextEnd), true);
		if(_MCFCRT_UTF_SUCCESS(c32CodePoint)){
			break;
		}
		dwErrorCode = UnlockedPopulate(Si);
		if(dwErrorCode != 0){
			UnlockAndSetLastError(Si, dwErrorCode);
			return -1;
		}
	}
	if(!bDontRemove){
		Si->uTextBegin = (size_t)((unsigned char *)pwcRead - Si->pbyBuffer);
	}
	Unlock(Si);
	return (long)c32CodePoint;
}
size_t _MCFCRT_ReadStandardInputText(wchar_t *restrict pwcText, size_t uLength, bool bStopAtEndOfLine, bool bDontRemove){
	_MCFCRT_FlushStandardOutput(false);

	if(!(Si->hFile)){
		SetLastError(ERROR_INVALID_HANDLE);
		return 0;
	}

	Lock(Si);
	const wchar_t *pwcRead;
	char32_t c32CodePoint;
	for(;;){
		DWORD dwErrorCode = UnlockedConvertBinaryToText(Si, false);
		if(dwErrorCode != 0){
			UnlockAndSetLastError(Si, dwErrorCode);
			return 0;
		}
		pwcRead = (void *)(Si->pbyBuffer + Si->uTextBegin);
		c32CodePoint = _MCFCRT_DecodeUtf16(&pwcRead, (void *)(Si->pbyBuffer + Si->uTextEnd), true);
		if(_MCFCRT_UTF_SUCCESS(c32CodePoint)){
			break;
		}
		dwErrorCode = UnlockedPopulate(Si);
		if(dwErrorCode != 0){
			UnlockAndSetLastError(Si, dwErrorCode);
			return 0;
		}
	}
	wchar_t *pwcWrite = pwcText;
	if(!_MCFCRT_UTF_SUCCESS(_MCFCRT_EncodeUtf16(&pwcWrite, pwcText + uLength, c32CodePoint, true))){
		UnlockAndSetLastError(Si, ERROR_INSUFFICIENT_BUFFER);
		return 0;
	}
	for(;;){
		c32CodePoint = _MCFCRT_DecodeUtf16(&pwcRead, (void *)(Si->pbyBuffer + Si->uTextEnd), true);
		if(!_MCFCRT_UTF_SUCCESS(c32CodePoint)){
			break;
		}
		if(!_MCFCRT_UTF_SUCCESS(_MCFCRT_EncodeUtf16(&pwcWrite, pwcText + uLength, c32CodePoint, true))){
			break;
		}
		if(bStopAtEndOfLine && (c32CodePoint == U'\n')){
			break;
		}
	}
	if(!bDontRemove){
		Si->uTextBegin = (size_t)((unsigned char *)pwcRead - Si->pbyBuffer);
	}
	Unlock(Si);
	return (size_t)(pwcWrite - pwcText);
}
int _MCFCRT_ReadStandardInputByte(bool bDontRemove){
	_MCFCRT_FlushStandardOutput(false);

	if(!(Si->hFile)){
		SetLastError(ERROR_INVALID_HANDLE);
		return -1;
	}

	Lock(Si);
	const unsigned char *pbyRead;
	size_t uBytesAvail;
	for(;;){
		DWORD dwErrorCode = UnlockedConvertTextToBinary(Si, false);
		if(dwErrorCode != 0){
			UnlockAndSetLastError(Si, dwErrorCode);
			return -1;
		}
		pbyRead = (void *)(Si->pbyBuffer + Si->uBinaryBegin);
		uBytesAvail = Si->uBinaryEnd - Si->uBinaryBegin;
		if(uBytesAvail > 0){
			break;
		}
		dwErrorCode = UnlockedPopulate(Si);
		if(dwErrorCode != 0){
			UnlockAndSetLastError(Si, dwErrorCode);
			return -1;
		}
	}
	const int nByteRead = *pbyRead;
	pbyRead += 1;
	if(!bDontRemove){
		Si->uBinaryBegin = (size_t)((unsigned char *)pbyRead - Si->pbyBuffer);
	}
	Unlock(Si);
	return nByteRead;
}
size_t _MCFCRT_ReadStandardInputBinary(void *restrict pData, size_t uSize, bool bDontRemove){
	_MCFCRT_FlushStandardOutput(false);

	if(!(Si->hFile)){
		SetLastError(ERROR_INVALID_HANDLE);
		return 0;
	}

	Lock(Si);
	const unsigned char *pbyRead;
	size_t uBytesAvail;
	for(;;){
		DWORD dwErrorCode = UnlockedConvertTextToBinary(Si, false);
		if(dwErrorCode != 0){
			UnlockAndSetLastError(Si, dwErrorCode);
			return 0;
		}
		pbyRead = (void *)(Si->pbyBuffer + Si->uBinaryBegin);
		uBytesAvail = Si->uBinaryEnd - Si->uBinaryBegin;
		if(uBytesAvail > 0){
			break;
		}
		dwErrorCode = UnlockedPopulate(Si);
		if(dwErrorCode != 0){
			UnlockAndSetLastError(Si, dwErrorCode);
			return 0;
		}
	}
	if(uSize == 0){
		UnlockAndSetLastError(Si, ERROR_INSUFFICIENT_BUFFER);
		return 0;
	}
	const size_t uBytesRead = (uSize < uBytesAvail) ? uSize : uBytesAvail;
	memcpy(pData, pbyRead, uBytesRead);
	pbyRead += uBytesRead;
	if(!bDontRemove){
		Si->uBinaryBegin = (size_t)((unsigned char *)pbyRead - Si->pbyBuffer);
	}
	Unlock(Si);
	return uBytesRead;
}

bool _MCFCRT_IsStandardInputEchoing(void){
	if(!(Si->hFile)){
		// SetLastError(ERROR_INVALID_HANDLE);
		return false;
	}

	Lock(Si);
	const bool bWasEchoing = Si->bEchoing;
	Unlock(Si);
	return bWasEchoing;
}
int _MCFCRT_SetStandardInputEchoing(bool bEchoing){
	if(!(Si->hFile)){
		SetLastError(ERROR_INVALID_HANDLE);
		return -1;
	}

	Lock(Si);
	const bool bWasEchoing = Si->bEchoing;
	const DWORD dwEchoMode = ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT;
	DWORD dwErrorCode = bEchoing ? UnlockedSetConsoleMode(Si, 0, dwEchoMode)
	                             : UnlockedSetConsoleMode(Si, dwEchoMode, 0);
	if(dwErrorCode != 0){
		UnlockAndSetLastError(Si, dwErrorCode);
		return -1;
	}
	Si->bEchoing = bEchoing;
	Unlock(Si);
	return bWasEchoing;
}

bool _MCFCRT_WriteStandardOutputChar32(char32_t c32CodePoint){
	if(!(So->hFile)){
		SetLastError(ERROR_INVALID_HANDLE);
		return false;
	}

	Lock(So);
	DWORD dwErrorCode = UnlockedConvertBinaryToText(So, true);
	if(dwErrorCode != 0){
		UnlockAndSetLastError(So, dwErrorCode);
		return false;
	}
	dwErrorCode = UnlockedReserve(So, 2 * sizeof(wchar_t), 0);
	if(dwErrorCode != 0){
		UnlockAndSetLastError(So, dwErrorCode);
		return false;
	}
	bool bFlushNow = !(So->bBuffered);
	wchar_t *pwcWrite = (void *)(So->pbyBuffer + So->uTextEnd);
	_MCFCRT_UncheckedEncodeUtf16(&pwcWrite, c32CodePoint, true);
	bFlushNow |= c32CodePoint == U'\n';
	So->uTextEnd = (size_t)((unsigned char *)pwcWrite - So->pbyBuffer);
	bFlushNow |= So->uTextEnd - So->uTextBegin >= FLUSH_THRESHOLD;
	if(bFlushNow){
		UnlockedFlush(So, false);
	}
	Unlock(So);
	return true;
}
bool _MCFCRT_WriteStandardOutputText(const wchar_t *restrict pwcText, size_t uLength, bool bEndOfLine){
	if(!(So->hFile)){
		SetLastError(ERROR_INVALID_HANDLE);
		return false;
	}

	Lock(So);
	DWORD dwErrorCode = UnlockedConvertBinaryToText(So, true);
	if(dwErrorCode != 0){
		UnlockAndSetLastError(So, dwErrorCode);
		return false;
	}
	dwErrorCode = UnlockedReserve(So, uLength * sizeof(wchar_t) + sizeof(wchar_t), 0);
	if(dwErrorCode != 0){
		UnlockAndSetLastError(So, dwErrorCode);
		return false;
	}
	bool bFlushNow = !(So->bBuffered);
	const wchar_t *pwcRead = pwcText;
	const wchar_t *const pwcReadEnd = pwcText + uLength;
	wchar_t *pwcWrite = (void *)(So->pbyBuffer + So->uTextEnd);
	for(;;){
		char32_t c32CodePoint = _MCFCRT_DecodeUtf16(&pwcRead, pwcReadEnd, true);
		if(!_MCFCRT_UTF_SUCCESS(c32CodePoint)){
			break;
		}
		_MCFCRT_UncheckedEncodeUtf16(&pwcWrite, c32CodePoint, true);
		bFlushNow |= c32CodePoint == U'\n';
	}
	while(pwcRead != pwcReadEnd){
		_MCFCRT_UncheckedEncodeUtf16(&pwcWrite, (uint16_t)*(pwcRead++), true);
	}
	if(bEndOfLine){
		_MCFCRT_UncheckedEncodeUtf16(&pwcWrite, U'\n', true);
		bFlushNow = true;
	}
	So->uTextEnd = (size_t)((unsigned char *)pwcWrite - So->pbyBuffer);
	bFlushNow |= So->uTextEnd - So->uTextBegin >= FLUSH_THRESHOLD;
	if(bFlushNow){
		UnlockedFlush(So, false);
	}
	Unlock(So);
	return true;
}
bool _MCFCRT_WriteStandardOutputByte(unsigned char byData){
	if(!(So->hFile)){
		SetLastError(ERROR_INVALID_HANDLE);
		return false;
	}

	Lock(So);
	DWORD dwErrorCode = UnlockedConvertTextToBinary(So, true);
	if(dwErrorCode != 0){
		UnlockAndSetLastError(So, dwErrorCode);
		return false;
	}
	dwErrorCode = UnlockedReserve(So, 0, 1);
	if(dwErrorCode != 0){
		UnlockAndSetLastError(So, dwErrorCode);
		return false;
	}
	bool bFlushNow = !(So->bBuffered);
	unsigned char *pbyWrite = (void *)(So->pbyBuffer + So->uBinaryEnd);
	*pbyWrite = byData;
	pbyWrite += 1;
	So->uBinaryEnd = (size_t)((unsigned char *)pbyWrite - So->pbyBuffer);
	bFlushNow |= So->uTextEnd - So->uTextBegin >= FLUSH_THRESHOLD;
	if(bFlushNow){
		UnlockedFlush(So, false);
	}
	Unlock(So);
	return true;
}
bool _MCFCRT_WriteStandardOutputBinary(const void *restrict pData, size_t uSize){
	if(!(So->hFile)){
		SetLastError(ERROR_INVALID_HANDLE);
		return false;
	}

	Lock(So);
	DWORD dwErrorCode = UnlockedConvertTextToBinary(So, true);
	if(dwErrorCode != 0){
		UnlockAndSetLastError(So, dwErrorCode);
		return false;
	}
	dwErrorCode = UnlockedReserve(So, 0, uSize);
	if(dwErrorCode != 0){
		UnlockAndSetLastError(So, dwErrorCode);
		return false;
	}
	bool bFlushNow = !(So->bBuffered);
	unsigned char *pbyWrite = (void *)(So->pbyBuffer + So->uBinaryEnd);
	memcpy(pbyWrite, pData, uSize);
	pbyWrite += uSize;
	So->uBinaryEnd = (size_t)((unsigned char *)pbyWrite - So->pbyBuffer);
	bFlushNow |= So->uTextEnd - So->uTextBegin >= FLUSH_THRESHOLD;
	if(bFlushNow){
		UnlockedFlush(So, false);
	}
	Unlock(So);
	return true;
}
bool _MCFCRT_FlushStandardOutput(bool bHard){
	if(!(So->hFile)){
		SetLastError(ERROR_INVALID_HANDLE);
		return false;
	}

	Lock(So);
	DWORD dwErrorCode = UnlockedFlush(So, bHard);
	if(dwErrorCode != 0){
		UnlockAndSetLastError(So, dwErrorCode);
		return false;
	}
	Unlock(So);
	return true;
}

bool _MCFCRT_IsStandardOutputBuffered(void){
	if(!(So->hFile)){
		// SetLastError(ERROR_INVALID_HANDLE);
		return false;
	}

	Lock(So);
	const bool bWasBuffered = So->bBuffered;
	Unlock(So);
	return bWasBuffered;
}
int _MCFCRT_SetStandardOutputBuffered(bool bBuffered){
	if(!(So->hFile)){
		SetLastError(ERROR_INVALID_HANDLE);
		return -1;
	}

	Lock(So);
	const bool bWasBuffered = So->bBuffered;
	if(bWasBuffered && !bBuffered){
		UnlockedFlush(So, false);
	}
	So->bBuffered = bBuffered;
	Unlock(So);
	return bWasBuffered;
}

bool _MCFCRT_WriteStandardErrorChar32(char32_t c32CodePoint){
	_MCFCRT_FlushStandardOutput(false);

	if(!(Se->hFile)){
		SetLastError(ERROR_INVALID_HANDLE);
		return false;
	}

	Lock(Se);
	DWORD dwErrorCode = UnlockedConvertBinaryToText(Se, true);
	if(dwErrorCode != 0){
		UnlockAndSetLastError(Se, dwErrorCode);
		return false;
	}
	dwErrorCode = UnlockedReserve(Se, 2 * sizeof(wchar_t), 0);
	if(dwErrorCode != 0){
		UnlockAndSetLastError(Se, dwErrorCode);
		return false;
	}
	wchar_t *pwcWrite = (void *)(Se->pbyBuffer + Se->uTextEnd);
	_MCFCRT_UncheckedEncodeUtf16(&pwcWrite, c32CodePoint, true);
	Se->uTextEnd = (size_t)((unsigned char *)pwcWrite - Se->pbyBuffer);
	UnlockedFlush(Se, false);
	Unlock(Se);
	return true;
}
bool _MCFCRT_WriteStandardErrorText(const wchar_t *restrict pwcText, size_t uLength, bool bEndOfLine){
	_MCFCRT_FlushStandardOutput(false);

	if(!(Se->hFile)){
		SetLastError(ERROR_INVALID_HANDLE);
		return false;
	}

	Lock(Se);
	DWORD dwErrorCode = UnlockedConvertBinaryToText(Se, true);
	if(dwErrorCode != 0){
		UnlockAndSetLastError(Se, dwErrorCode);
		return false;
	}
	dwErrorCode = UnlockedReserve(Se, uLength * sizeof(wchar_t) + sizeof(wchar_t), 0);
	if(dwErrorCode != 0){
		UnlockAndSetLastError(Se, dwErrorCode);
		return false;
	}
	const wchar_t *pwcRead = pwcText;
	const wchar_t *const pwcReadEnd = pwcText + uLength;
	wchar_t *pwcWrite = (void *)(Se->pbyBuffer + Se->uTextEnd);
	for(;;){
		char32_t c32CodePoint = _MCFCRT_DecodeUtf16(&pwcRead, pwcReadEnd, true);
		if(!_MCFCRT_UTF_SUCCESS(c32CodePoint)){
			break;
		}
		_MCFCRT_UncheckedEncodeUtf16(&pwcWrite, c32CodePoint, true);
	}
	while(pwcRead != pwcReadEnd){
		_MCFCRT_UncheckedEncodeUtf16(&pwcWrite, (uint16_t)*(pwcRead++), true);
	}
	if(bEndOfLine){
		_MCFCRT_UncheckedEncodeUtf16(&pwcWrite, U'\n', true);
	}
	Se->uTextEnd = (size_t)((unsigned char *)pwcWrite - Se->pbyBuffer);
	UnlockedFlush(Se, false);
	Unlock(Se);
	return true;
}
bool _MCFCRT_WriteStandardErrorByte(unsigned char byData){
	_MCFCRT_FlushStandardOutput(false);

	if(!(Se->hFile)){
		SetLastError(ERROR_INVALID_HANDLE);
		return false;
	}

	Lock(Se);
	DWORD dwErrorCode = UnlockedConvertTextToBinary(Se, true);
	if(dwErrorCode != 0){
		UnlockAndSetLastError(Se, dwErrorCode);
		return false;
	}
	dwErrorCode = UnlockedReserve(Se, 0, 1);
	if(dwErrorCode != 0){
		UnlockAndSetLastError(Se, dwErrorCode);
		return false;
	}
	unsigned char *pbyWrite = (void *)(Se->pbyBuffer + Se->uBinaryEnd);
	*pbyWrite = byData;
	pbyWrite += 1;
	Se->uBinaryEnd = (size_t)((unsigned char *)pbyWrite - Se->pbyBuffer);
	UnlockedFlush(Se, false);
	Unlock(Se);
	return true;
}
bool _MCFCRT_WriteStandardErrorBinary(const void *restrict pData, size_t uSize){
	_MCFCRT_FlushStandardOutput(false);

	if(!(Se->hFile)){
		SetLastError(ERROR_INVALID_HANDLE);
		return false;
	}

	Lock(Se);
	DWORD dwErrorCode = UnlockedConvertTextToBinary(Se, true);
	if(dwErrorCode != 0){
		UnlockAndSetLastError(Se, dwErrorCode);
		return false;
	}
	dwErrorCode = UnlockedReserve(Se, 0, uSize);
	if(dwErrorCode != 0){
		UnlockAndSetLastError(Se, dwErrorCode);
		return false;
	}
	unsigned char *pbyWrite = (void *)(Se->pbyBuffer + Se->uBinaryEnd);
	memcpy(pbyWrite, pData, uSize);
	pbyWrite += uSize;
	Se->uBinaryEnd = (size_t)((unsigned char *)pbyWrite - Se->pbyBuffer);
	UnlockedFlush(Se, false);
	Unlock(Se);
	return true;
}
