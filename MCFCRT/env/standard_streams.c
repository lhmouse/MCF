// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "standard_streams.h"
#include "../ext/assert.h"
#include "../ext/utf.h"
#include "mutex.h"
#include "mcfwin.h"
/*
typedef struct tagStream {
	HANDLE hFile;
	bool bConsole;

	_MCFCRT_Mutex vMutex;
	bool bShutdown;
	unsigned uBufferBegin;
	unsigned uBufferEnd;
	unsigned char alignas(16) abyBuffer[4096];
} Stream;

static void OpenStream(Stream *pStream, DWORD dwSlot){
	const HANDLE hFile = GetStdHandle(dwSlot);
	_MCFCRT_ASSERT(hFile != INVALID_HANDLE_VALUE);

	bool bConsole = false;
	DWORD dwMode;
	if(GetConsoleMode(hFile, &dwMode)){
		bConsole = true;
	}

	pStream->hFile        = hFile;
	pStream->bConsole     = bConsole;

	pStream->bShutdown    = false;
	pStream->uBufferBegin = 0;
	pStream->uBufferEnd   = 0;
}
static void ShutdownStream(Stream *pStream){
//FlushFileBuffers
	pStream->bShutdown = true;
}

static Stream g_vStdIn  = { 0 };
static Stream g_vStdOut = { 0 };
static Stream g_vStdErr = { 0 };

bool __MCFCRT_StandardStreamsInit(void){
	OpenStream(&g_vStdOut, STD_OUTPUT_HANDLE);
	OpenStream(&g_vStdErr, STD_ERROR_HANDLE );
	OpenStream(&g_vStdIn , STD_INPUT_HANDLE );
	return true;
}
void __MCFCRT_StandardStreamsUninit(void){
	ShutdownStream(&g_vStdIn );
	ShutdownStream(&g_vStdErr);
	ShutdownStream(&g_vStdOut);
}

size_t _MCFCRT_PeekStandardInputString(wchar_t *pwcString, size_t uLength, bool bSingleLine){
}
size_t _MCFCRT_PeekStandardInputBinary(void *pBuffer, size_t uSize){
}
size_t _MCFCRT_ReadStandardInputString(wchar_t *pwcString, size_t uLength, bool bSingleLine){
}
size_t _MCFCRT_ReadStandardInputBinary(void *pBuffer, size_t uSize){
}
size_t _MCFCRT_DiscardStandardInputString(size_t uLength, bool bSingleLine){
}
size_t _MCFCRT_DiscardStandardInputBinary(size_t uSize){
}
bool _MCFCRT_IsStandardInputEchoing(void){
}
bool _MCFCRT_SetStandardInputEchoing(bool bEchoing){
}

size_t _MCFCRT_WriteStandardOutputString(const wchar_t *pwcString, size_t uLength, bool bAppendNewLine){
}
size_t _MCFCRT_WriteStandardOutputBinary(const void *pBuffer, size_t uSize){
}
bool _MCFCRT_IsStandardOutputBuffered(void){
}
bool _MCFCRT_SetStandardOutputBuffered(bool bBuffered){
}
bool _MCFCRT_FlushStandardOutput(bool bHard){
}

size_t _MCFCRT_WriteStandardErrorString(const wchar_t *pwcString, size_t uLength, bool bAppendNewLine){
}
size_t _MCFCRT_WriteStandardErrorBinary(const void *pBuffer, size_t uSize){
}*/
