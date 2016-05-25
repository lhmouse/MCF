// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "standard_streams.h"
#include "../ext/assert.h"
#include "../ext/utf.h"
#include "mutex.h"
#include "mcfwin.h"
#include <stdlib.h>

typedef struct tagStreamControl {
	HANDLE hFile;
	bool bConsole;
	bool bThrottled;

	unsigned char *pbyBuffer;
	size_t uCapacity;
	size_t uBegin;
	size_t uEnd;
} StreamControl;

static bool ReserveBuffer(StreamControl *pStream, size_t uMinSizeReserved){
	if(pStream->uCapacity - pStream->uEnd >= uMinSizeReserved){
		return true;
	}

	const size_t uSize = pStream->uEnd - pStream->uBegin;
	if(pStream->uCapacity - uSize >= uMinSizeReserved){
		memmove(pStream->pbyBuffer, pStream->pbyBuffer + pStream->uBegin, uSize);
		pStream->uBegin = 0;
		pStream->uEnd   = uSize;
		return true;
	}

	const size_t uMinCapacity = uSize + uMinSizeReserved;
	if(uMinCapacity < uSize){
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return false;
	}
	size_t uNewCapacity = uMinCapacity;
	uNewCapacity += (uNewCapacity >> 1);
	uNewCapacity = (uNewCapacity + 0x0F) & (size_t)-0x10;
	if(uNewCapacity < uMinCapacity){
		uNewCapacity = uMinCapacity;
	}
	unsigned char *const pbyNewBuffer = malloc(uNewCapacity);
	if(!pbyNewBuffer){
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return false;
	}
	memmove(pbyNewBuffer, pStream->pbyBuffer + pStream->uBegin, uSize);
	free(pStream->pbyBuffer);
	pStream->pbyBuffer = pbyNewBuffer;
	pStream->uCapacity = uNewCapacity;
	pStream->uBegin    = 0;
	pStream->uEnd      = uSize;
	return true;
}
static bool PopulateInputBuffer(StreamControl *pStream){
}
static bool FlushOutputBuffer(StreamControl *pStream){
}

static void ResetStream(StreamControl *pStream, DWORD dwSlot){
	HANDLE hFile = GetStdHandle(dwSlot);
	if(hFile == INVALID_HANDLE_VALUE){
		hFile = nullptr;
	}

	bool bConsole = false;
	if(hFile){
		DWORD dwMode;
		if(GetConsoleMode(hFile, &dwMode)){
			bConsole = true;
		}
	}

	pStream->hFile      = hFile;
	pStream->bConsole   = bConsole;
	pStream->bThrottled = false;
}
static void ThrottleStream(StreamControl *pStream, DWORD dwSlot){
	if(dwSlot != STD_INPUT_HANDLE){
		FlushOutputBuffer(pStream);
	}

	pStream->bThrottled = true;

	free(pStream->pbyBuffer);
	pStream->pbyBuffer = nullptr;
	pStream->uCapacity = 0;
	pStream->uBegin    = 0;
	pStream->uEnd      = 0;
}

static _MCFCRT_Mutex g_vStdInMutex   = { 0 };
static StreamControl g_vStdInStream  = { 0 };

static _MCFCRT_Mutex g_vStdOutMutex  = { 0 };
static StreamControl g_vStdOutStream = { 0 };

static _MCFCRT_Mutex g_vStdErrMutex  = { 0 };
static StreamControl g_vStdErrStream = { 0 };

bool __MCFCRT_StandardStreamsInit(void){
	ResetStream(&g_vStdOutStream, STD_OUTPUT_HANDLE);
	ResetStream(&g_vStdErrStream, STD_ERROR_HANDLE);
	ResetStream(&g_vStdInStream,  STD_INPUT_HANDLE);
	return true;
}
void __MCFCRT_StandardStreamsUninit(void){
	ThrottleStream(&g_vStdInStream,  STD_INPUT_HANDLE);
	ThrottleStream(&g_vStdErrStream, STD_ERROR_HANDLE);
	ThrottleStream(&g_vStdOutStream, STD_OUTPUT_HANDLE);
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
	return uSize;
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
	return uSize;
}
bool _MCFCRT_WriteStandardErrorChar32(char32_t c32CodePoint){
	return true;
}
bool _MCFCRT_WriteStandardErrorString(const wchar_t *pwcString, size_t uLength, bool bAppendNewLine){
	return uLength + bAppendNewLine;
}
