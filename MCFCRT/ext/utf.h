// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_EXT_UTF_H_
#define __MCFCRT_EXT_UTF_H_

#include "../env/_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

extern char32_t _MCFCRT_DecodeUtf8 (const char     **__ppchRead ) _MCFCRT_NOEXCEPT;
extern char32_t _MCFCRT_DecodeUtf16(const char16_t **__ppc16Read) _MCFCRT_NOEXCEPT;

extern void _MCFCRT_EncodeUtf8 (char     **__ppchWrite , char32_t __c32CodePoint) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_EncodeUtf16(char16_t **__ppc16Write, char32_t __c32CodePoint) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
