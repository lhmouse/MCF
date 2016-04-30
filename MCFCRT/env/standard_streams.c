// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "standard_streams.h"
#include "../ext/assert.h"
#include "../ext/utf.h"
#include "mutex.h"
#include "mcfwin.h"

bool __MCFCRT_StandardStreamsInit(void){
	return true;
}
void __MCFCRT_StandardStreamsUninit(void){
}

ptrdiff_t _MCFCRT_PeekStandardInputAsText(wchar_t *pwcString, size_t uLength, bool bSingleLine){
}
ptrdiff_t _MCFCRT_PeekStandardInputAsBinary(void *pBuffer, size_t uSize){
}
ptrdiff_t _MCFCRT_ReadStandardInputAsText(wchar_t *pwcString, size_t uLength, bool bSingleLine){
}
ptrdiff_t _MCFCRT_ReadStandardInputAsBinary(void *pBuffer, size_t uSize){
}
ptrdiff_t _MCFCRT_DiscardStandardInputAsText(size_t uLength, bool bSingleLine){
}
ptrdiff_t _MCFCRT_DiscardStandardInputAsBinary(size_t uSize){
}
bool _MCFCRT_IsStandardInputEchoing(void){
}
bool _MCFCRT_SetStandardInputEchoing(bool bEchoing){
}

ptrdiff_t _MCFCRT_WriteStandardOutputAsText(const wchar_t *pwcString, size_t uLength, bool bAppendNewLine){
}
ptrdiff_t _MCFCRT_WriteStandardOutputAsBinary(const void *pBuffer, size_t uSize){
}
bool _MCFCRT_IsStandardOutputBuffered(void){
}
bool _MCFCRT_SetStandardOutputBuffered(bool bBuffered){
}
bool _MCFCRT_FlushStandardOutput(bool bHard){
}

ptrdiff_t _MCFCRT_WriteStandardErrorAsText(const wchar_t *pwcString, size_t uLength, bool bAppendNewLine){
}
ptrdiff_t _MCFCRT_WriteStandardErrorAsBinary(const void *pBuffer, size_t uSize){
}
