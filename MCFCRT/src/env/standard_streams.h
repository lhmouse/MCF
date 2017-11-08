// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_STANDARD_STREAMS_H_
#define __MCFCRT_ENV_STANDARD_STREAMS_H_

#include "_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

extern bool __MCFCRT_StandardStreamsInit(void) _MCFCRT_NOEXCEPT;
extern void __MCFCRT_StandardStreamsUninit(void) _MCFCRT_NOEXCEPT;

// 1. A stream may be a console or a non-console redirected from/to a file, a pipe, etc.
// 2. When operating a console in binary format or a non-console in text format, data are converted between UTF-8 and UTF-16.
// 3. When reading in text format, no function returns truncated code points. Invalid code points are replaced with U+FFFD.
// 4. When writing in text format, strings passed to individual function calls must start and end in initial shift states. Truncated or invalid code points are replaced with U+FFFD.
// 6. In all other cases, data are copied as-is. No integrity check is made.

// Standard Input
extern long _MCFCRT_ReadStandardInputChar32(void) _MCFCRT_NOEXCEPT;
extern _MCFCRT_STD size_t _MCFCRT_ReadStandardInputText(wchar_t *_MCFCRT_RESTRICT __pwcText, _MCFCRT_STD size_t __uLength, bool __bStopAtEndOfLine) _MCFCRT_NOEXCEPT;
extern int _MCFCRT_ReadStandardInputByte(void) _MCFCRT_NOEXCEPT;
extern _MCFCRT_STD size_t _MCFCRT_ReadStandardInputBinary(void *_MCFCRT_RESTRICT __pData, _MCFCRT_STD size_t __uSize) _MCFCRT_NOEXCEPT;

extern bool _MCFCRT_IsStandardInputEchoing(void) _MCFCRT_NOEXCEPT;
extern int _MCFCRT_SetStandardInputEchoing(bool __bEchoing) _MCFCRT_NOEXCEPT; // Returns the previous state or -1 in case of failure.

// Standard Output
extern bool _MCFCRT_WriteStandardOutputChar32(char32_t __c32CodePoint) _MCFCRT_NOEXCEPT;
extern bool _MCFCRT_WriteStandardOutputText(const wchar_t *_MCFCRT_RESTRICT __pwcText, _MCFCRT_STD size_t __uLength, bool __bEndOfLine) _MCFCRT_NOEXCEPT;
extern bool _MCFCRT_WriteStandardOutputByte(unsigned char __byData) _MCFCRT_NOEXCEPT;
extern bool _MCFCRT_WriteStandardOutputBinary(const void *_MCFCRT_RESTRICT __pData, _MCFCRT_STD size_t __uSize) _MCFCRT_NOEXCEPT;
extern bool _MCFCRT_FlushStandardOutput(bool __bHard) _MCFCRT_NOEXCEPT;

extern bool _MCFCRT_IsStandardOutputBuffered(void) _MCFCRT_NOEXCEPT;
extern int _MCFCRT_SetStandardOutputBuffered(bool __bBuffered) _MCFCRT_NOEXCEPT; // Returns the previous state or -1 in case of failure.

// Standard Error
extern bool _MCFCRT_WriteStandardErrorChar32(char32_t __c32CodePoint) _MCFCRT_NOEXCEPT;
extern bool _MCFCRT_WriteStandardErrorText(const wchar_t *_MCFCRT_RESTRICT __pwcText, _MCFCRT_STD size_t __uLength, bool __bEndOfLine) _MCFCRT_NOEXCEPT;
extern bool _MCFCRT_WriteStandardErrorByte(unsigned char __byData) _MCFCRT_NOEXCEPT;
extern bool _MCFCRT_WriteStandardErrorBinary(const void *_MCFCRT_RESTRICT __pData, _MCFCRT_STD size_t __uSize) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
