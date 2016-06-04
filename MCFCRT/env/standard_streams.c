// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "standard_streams.h"
#include "../ext/assert.h"
#include "../ext/utf.h"
#include "mutex.h"
#include "mcfwin.h"
#include "heap.h"

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
	return true;
}
