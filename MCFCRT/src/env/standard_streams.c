// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "standard_streams.h"
#include "../ext/assert.h"
#include "../ext/utf.h"
#include "mutex.h"
#include "mcfwin.h"
/*
typedef struct __MCFCRT_tagContiguousBuffer {
	void *__pStorageBegin;
	void *__pDataBegin;
	void *__pDataEnd;
	void *__pStorageEnd;
} _MCFCRT_ContiguousBuffer;

bool _MCFCRT_ContiguousBufferReserve(_MCFCRT_ContiguousBuffer *restrict pBuffer, void **restrict ppData, size_t *restrict puSizeReserved, size_t uSizeToReserve){
	char *pchStorageBegin = pBuffer->__pStorageBegin;
	char *pchDataBegin    = pBuffer->__pDataBegin;
	char *pchDataEnd      = pBuffer->__pDataEnd;
	char *pchStorageEnd   = pBuffer->__pStorageEnd;

	if(uSizeToReserve <= (size_t)(pchStorageEnd - pchDataEnd)){
		// 无事可做。
	} else {
		const size_t uOldSize = (size_t)(pchDataEnd - pchDataBegin);
		const size_t uMinNewCapacity = uOldSize + uSizeToReserve;
		if(uMinNewCapacity < uOldSize){
			return false;
		}
		const size_t uOldCapacity = (size_t)(pchStorageEnd - pchStorageBegin);
		if(uMinNewCapacity <= uOldCapacity){
			memmove(pchStorageBegin, pchDataBegin, uOldSize);

			pchDataBegin = pchStorageBegin;
			pchDataEnd   = pchDataBegin + uOldSize;

			pBuffer->__pDataBegin = pchDataBegin;
			pBuffer->__pDataEnd   = pchDataEnd;
		} else {
			size_t uNewCapacity = uOldCapacity + 1;
			uNewCapacity += (uNewCapacity >> 1);
			uNewCapacity = (uNewCapacity + 0x0F) & (size_t)-0x10;
			if(uNewCapacity < uMinNewCapacity){
				uNewCapacity = uMinNewCapacity;
			}
			char *const pchNewStorage = _MCFCRT_malloc(uNewCapacity);
			if(!pchNewStorage){
				return false;
			}
			memcpy(pchNewStorage, pchDataBegin, uOldSize);
			_MCFCRT_free(pchStorageBegin);

			pchStorageBegin = pchNewStorage;
			pchDataBegin    = pchNewStorage;
			pchDataEnd      = pchDataBegin + uOldSize;
			pchStorageEnd   = pchStorageBegin + uNewCapacity;

			pBuffer->__pStorageBegin = pchStorageBegin;
			pBuffer->__pDataBegin    = pchDataBegin;
			pBuffer->__pDataEnd      = pchDataEnd;
			pBuffer->__pStorageEnd   = pchStorageEnd;
		}
	}
	_MCFCRT_ASSERT(uSizeToReserve <= (size_t)(pchStorageEnd - pchDataEnd));

	*ppData         = pchDataEnd;
	*puSizeReserved = (size_t)(pchStorageEnd - pchDataEnd);
	return true;
}
void _MCFCRT_ContiguousBufferRecycle(_MCFCRT_ContiguousBuffer *restrict pBuffer){
	char *const pchStorageBegin = pBuffer->__pStorageBegin;

	_MCFCRT_free(pchStorageBegin);

	pBuffer->__pStorageBegin = nullptr;
	pBuffer->__pDataBegin    = nullptr;
	pBuffer->__pDataEnd      = nullptr;
	pBuffer->__pStorageEnd   = nullptr;
}

static inline void _MCFCRT_ContiguousBufferPeek(const _MCFCRT_ContiguousBuffer *restrict __pBuffer, void **restrict __ppData, _MCFCRT_STD size_t *restrict __puSize) _MCFCRT_NOEXCEPT {
	char *const __pchDataBegin = (char *)__pBuffer->__pDataBegin;
	char *const __pchDataEnd   = (char *)__pBuffer->__pDataEnd;

	*__ppData = __pchDataBegin;
	*__puSize = (_MCFCRT_STD size_t)(__pchDataEnd - __pchDataBegin);
}
static inline void _MCFCRT_ContiguousBufferDiscard(_MCFCRT_ContiguousBuffer *restrict __pBuffer, _MCFCRT_STD size_t __uSizeToDiscard) _MCFCRT_NOEXCEPT {
	char *const __pchDataBegin = (char *)__pBuffer->__pDataBegin;
	char *const __pchDataEnd   = (char *)__pBuffer->__pDataEnd;

	_MCFCRT_ASSERT(__uSizeToDiscard <= (_MCFCRT_STD size_t)(__pchDataEnd - __pchDataBegin));

	__pBuffer->__pDataBegin = __pchDataBegin + __uSizeToDiscard;
}
static inline void _MCFCRT_ContiguousBufferAdopt(_MCFCRT_ContiguousBuffer *restrict __pBuffer, _MCFCRT_STD size_t __uSizeToAdopt) _MCFCRT_NOEXCEPT {
	char *const __pchDataEnd    = (char *)__pBuffer->__pDataEnd;
	char *const __pchStorageEnd = (char *)__pBuffer->__pStorageEnd;

	_MCFCRT_ASSERT(__uSizeToAdopt <= (_MCFCRT_STD size_t)(__pchStorageEnd - __pchDataEnd));

	__pBuffer->__pDataEnd = __pchDataEnd + __uSizeToAdopt;
}
typedef struct tagStreamControl {
	_MCFCRT_Mutex vMutex;
	HANDLE hPipe;
	bool bConsole;
	bool bBuffering;
	_MCFCRT_ContiguousBuffer vBuffer;
} StreamControl;

static StreamControl g_scStdIn, g_scStdOut, g_scStdErr;

static void FlushStream(StreamControl *pControl){
}

static void InitializeStream(StreamControl *pControl, DWORD dwSlot){
	//
}
static void UninitializeStream(StreamControl *pControl){
	FlushStream(pControl);

	pControl->bBuffering = false;
	_MCFCRT_ContiguousBufferRecycle(&(pControl->vBuffer));
}
*/
bool __MCFCRT_StandardStreamsInit(void){
//	InitializeStream(&g_scStdErr, STD_ERROR_HANDLE);
//	InitializeStream(&g_scStdOut, STD_OUTPUT_HANDLE);
//	InitializeStream(&g_scStdIn, STD_INPUT_HANDLE);
	return true;
}
void __MCFCRT_StandardStreamsUninit(void){
//	UninitializeStream(&g_scStdIn);
//	UninitializeStream(&g_scStdOut);
//	UninitializeStream(&g_scStdErr);
}

int _MCFCRT_PeekStandardInputByte(void){
	SetLastError(ERROR_HANDLE_EOF);
	return -1;
}
int _MCFCRT_ReadStandardInputByte(void){
	SetLastError(ERROR_HANDLE_EOF);
	return -1;
}
size_t _MCFCRT_PeekStandardInputBinary(void *pBuffer, size_t uSize){
	SetLastError(ERROR_HANDLE_EOF);
	return 0;
}
size_t _MCFCRT_ReadStandardInputBinary(void *pBuffer, size_t uSize){
	SetLastError(ERROR_HANDLE_EOF);
	return 0;
}
size_t _MCFCRT_DiscardStandardInputBinary(size_t uSize){
	SetLastError(ERROR_HANDLE_EOF);
	return 0;
}
long _MCFCRT_PeekStandardInputChar32(void){
	SetLastError(ERROR_HANDLE_EOF);
	return -1;
}
long _MCFCRT_ReadStandardInputChar32(void){
	SetLastError(ERROR_HANDLE_EOF);
	return -1;
}
size_t _MCFCRT_PeekStandardInputString(wchar_t *pwcString, size_t uLength, bool bSingleLine){
	SetLastError(ERROR_HANDLE_EOF);
	return 0;
}
size_t _MCFCRT_ReadStandardInputString(wchar_t *pwcString, size_t uLength, bool bSingleLine){
	SetLastError(ERROR_HANDLE_EOF);
	return 0;
}
size_t _MCFCRT_DiscardStandardInputString(size_t uLength, bool bSingleLine){
	SetLastError(ERROR_HANDLE_EOF);
	return 0;
}
bool _MCFCRT_IsStandardInputEchoing(void){
	return true;
}
bool _MCFCRT_SetStandardInputEchoing(bool bEchoing){
	return true;
}

bool _MCFCRT_WriteStandardOutputByte(unsigned char byData){
	return true;
}
bool _MCFCRT_WriteStandardOutputBinary(const void *pBuffer, size_t uSize){
	return true;
}
bool _MCFCRT_WriteStandardOutputChar32(char32_t c32CodePoint){
	return true;
}
bool _MCFCRT_WriteStandardOutputString(const wchar_t *pwcString, size_t uLength, bool bAppendNewLine){
	return uLength + bAppendNewLine;
}
bool _MCFCRT_IsStandardOutputBuffered(void){
	return true;
}
bool _MCFCRT_SetStandardOutputBuffered(bool bBuffered){
	return true;
}
bool _MCFCRT_FlushStandardOutput(bool bHard){
	return true;
}

bool _MCFCRT_WriteStandardErrorByte(unsigned char byData){
	return true;
}
bool _MCFCRT_WriteStandardErrorBinary(const void *pBuffer, size_t uSize){
	return true;
}
bool _MCFCRT_WriteStandardErrorChar32(char32_t c32CodePoint){
	return true;
}
bool _MCFCRT_WriteStandardErrorString(const wchar_t *pwcString, size_t uLength, bool bAppendNewLine){
	HANDLE h = GetStdHandle(STD_ERROR_HANDLE);
	if(h){
		DWORD written;
		WriteConsoleW(h, pwcString, (DWORD)uLength, &written, nullptr);
	}
	return true;
}
