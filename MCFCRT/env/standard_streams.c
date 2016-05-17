// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "standard_streams.h"
#include "../ext/assert.h"
#include "../ext/utf.h"
#include "mutex.h"
#include "mcfwin.h"
/*
typedef struct tagChunkHeader {
	size_t uOffset;
	size_t uSize;
	unsigned char abyRedZone[16];
	unsigned char abyData[];
} ChunkHeader;

typedef struct tagStream {
	HANDLE hFile;
	bool bInteractive;

	bool bThrottled;
	ChunkHeader *pBuffedData;
} Stream;

static void Reset(Stream *pStream, DWORD dwSlot){
	HANDLE hFile = GetStdHandle(dwSlot);
	_MCFCRT_ASSERT(hFile != INVALID_HANDLE_VALUE);

	bool bInteractive = false;
	DWORD dwMode;
	if(GetConsoleMode(hFile, &dwMode)){
		bInteractive = true;
	}

	pStream->hFile        = hFile;
	pStream->bInteractive = bInteractive;

	pStream->bThrottled   = false;
	pStream->uBufferBegin = 0;
	pStream->uBufferEnd   = 0;
}


static _MCFCRT_Mutex g_vStdInMutex  = { 0 };
static Stream        g_vStdInStream;

static _MCFCRT_Mutex g_vStdOutMutex = { 0 };
static Stream        g_vStdOutStream;

static _MCFCRT_Mutex g_vStdErrMutex = { 0 };
static Stream        g_vStdErrStream;




typedef struct tagStream {
	HANDLE hFile;
	bool bConsole;

	bool bThrottled;
	unsigned uBufferBegin;
	unsigned uBufferEnd;
	unsigned char alignas(16) abyBuffer[4096];
	unsigned char abySafetyBumper[4];
} Stream;

// 通用。
static void Reset(Stream *pStream, DWORD dwSlot){
	const HANDLE hFile = GetStdHandle(dwSlot);
	_MCFCRT_ASSERT(hFile != INVALID_HANDLE_VALUE);

	bool bConsole = false;
	DWORD dwMode;
	if(GetConsoleMode(hFile, &dwMode)){
		bConsole = true;
	}

	pStream->hFile        = hFile;
	pStream->bConsole     = bConsole;

	pStream->bThrottled   = false;
	pStream->uBufferBegin = 0;
	pStream->uBufferEnd   = 0;
}
static void Throttle(Stream *pStream){
	Flush(pStream);

	pStream->bThrottled = true;
}

// 输入。
static void Populate(Stream *pStream){
}

// 输出。
static void Compact(Stream *pStream){
}
static bool Flush(Stream *pStream){
}

static Stream g_vStdIn  = { 0 };
static Stream g_vStdOut = { 0 };
static Stream g_vStdErr = { 0 };

static void LockedReset(Stream *pStream, DWORD dwSlot){
	_MCFCRT_WaitForMutexForever(&(pStream->vMutex), _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	Reset(pStream, dwSlot);
	_MCFCRT_SignalMutex(&(pStream->vMutex));
}
static void LockedThrottle(Stream *pStream){
	_MCFCRT_WaitForMutexForever(&(pStream->vMutex), _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	Throttle(pStream);
	_MCFCRT_SignalMutex(&(pStream->vMutex));
}
*/
bool __MCFCRT_StandardStreamsInit(void){
	return true;
}
void __MCFCRT_StandardStreamsUninit(void){
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
size_t _MCFCRT_WriteStandardOutputBinary(const void *pBuffer, size_t uSize){
	return uSize;
}
bool _MCFCRT_WriteStandardOutputChar32(char32_t c32CodePoint){
	return true;
}
size_t _MCFCRT_WriteStandardOutputString(const wchar_t *pwcString, size_t uLength, bool bAppendNewLine){
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
size_t _MCFCRT_WriteStandardErrorBinary(const void *pBuffer, size_t uSize){
	return uSize;
}
bool _MCFCRT_WriteStandardErrorChar32(char32_t c32CodePoint){
	return true;
}
size_t _MCFCRT_WriteStandardErrorString(const wchar_t *pwcString, size_t uLength, bool bAppendNewLine){
	return uLength + bAppendNewLine;
}
