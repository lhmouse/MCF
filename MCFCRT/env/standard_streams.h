// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_STANDARD_STREAMS_H_
#define __MCFCRT_ENV_STANDARD_STREAMS_H_

#include "_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

extern bool __MCFCRT_StandardStreamsInit(void) _MCFCRT_NOEXCEPT;
extern void __MCFCRT_StandardStreamsUninit(void) _MCFCRT_NOEXCEPT;

// 这些函数使用 UTF 编码。如果操作的是控制台，以 UTF-16 作为内码；否则，假定操作的是字节流，以 UTF-8 作为内码。
// 对于返回 ptrdiff_t 的函数，成功返回非负值，失败返回 -1。

extern _MCFCRT_STD ptrdiff_t _MCFCRT_PeekStandardInputAsText(wchar_t *__pwcString, _MCFCRT_STD size_t __uLength, bool __bSingleLine) _MCFCRT_NOEXCEPT;
extern _MCFCRT_STD ptrdiff_t _MCFCRT_PeekStandardInputAsBinary(void *__pBuffer, _MCFCRT_STD size_t __uSize) _MCFCRT_NOEXCEPT;
extern _MCFCRT_STD ptrdiff_t _MCFCRT_ReadStandardInputAsText(wchar_t *__pwcString, _MCFCRT_STD size_t __uLength, bool __bSingleLine) _MCFCRT_NOEXCEPT;
extern _MCFCRT_STD ptrdiff_t _MCFCRT_ReadStandardInputAsBinary(void *__pBuffer, _MCFCRT_STD size_t __uSize) _MCFCRT_NOEXCEPT;
extern _MCFCRT_STD ptrdiff_t _MCFCRT_DiscardStandardInputAsText(_MCFCRT_STD size_t __uLength, bool __bSingleLine) _MCFCRT_NOEXCEPT;
extern _MCFCRT_STD ptrdiff_t _MCFCRT_DiscardStandardInputAsBinary(_MCFCRT_STD size_t __uSize) _MCFCRT_NOEXCEPT;
extern bool _MCFCRT_IsStandardInputEchoing(void) _MCFCRT_NOEXCEPT;
extern bool _MCFCRT_SetStandardInputEchoing(bool __bEchoing) _MCFCRT_NOEXCEPT;

extern _MCFCRT_STD ptrdiff_t _MCFCRT_WriteStandardOutputAsText(const wchar_t *__pwcString, _MCFCRT_STD size_t __uLength, bool __bAppendNewLine) _MCFCRT_NOEXCEPT;
extern _MCFCRT_STD ptrdiff_t _MCFCRT_WriteStandardOutputAsBinary(const void *__pBuffer, _MCFCRT_STD size_t __uSize) _MCFCRT_NOEXCEPT;
extern bool _MCFCRT_IsStandardOutputBuffered(void) _MCFCRT_NOEXCEPT;
extern bool _MCFCRT_SetStandardOutputBuffered(bool __bBuffered) _MCFCRT_NOEXCEPT;
extern bool _MCFCRT_FlushStandardOutput(bool __bHard) _MCFCRT_NOEXCEPT;

extern _MCFCRT_STD ptrdiff_t _MCFCRT_WriteStandardErrorAsText(const wchar_t *__pwcString, _MCFCRT_STD size_t __uLength, bool __bAppendNewLine) _MCFCRT_NOEXCEPT;
extern _MCFCRT_STD ptrdiff_t _MCFCRT_WriteStandardErrorAsBinary(const void *__pBuffer, _MCFCRT_STD size_t __uSize) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
