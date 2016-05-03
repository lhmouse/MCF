// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_EXT_UTF_H_
#define __MCFCRT_EXT_UTF_H_

#include "../env/_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

// 这些函数并不对字符串结束符进行特殊处理（字符串结束符被当做普通字符）。
// 这些函数以运行效率为首要目标，因此没有进行错误检测。
// 为了防止访问越界，输入输出的缓冲区应当能够容纳合法的最大编码点。对于 UTF-8 而言这个值是 4 个编码单元（char），而对于 UTF-16 这个值是 2 个编码单元（char16_t）。

extern char32_t _MCFCRT_DecodeUtf8 (const char     **__ppchRead ) _MCFCRT_NOEXCEPT;
extern char32_t _MCFCRT_DecodeUtf16(const char16_t **__ppc16Read) _MCFCRT_NOEXCEPT;

extern char32_t _MCFCRT_EncodeUtf8 (char     **__ppchWrite , char32_t __c32CodePoint) _MCFCRT_NOEXCEPT;
extern char32_t _MCFCRT_EncodeUtf16(char16_t **__ppc16Write, char32_t __c32CodePoint) _MCFCRT_NOEXCEPT;

extern char32_t _MCFCRT_EncodeUtf8FromUtf16(char     **__ppchWrite , const char16_t **__ppc16Read) _MCFCRT_NOEXCEPT;
extern char32_t _MCFCRT_EncodeUtf16FromUtf8(char16_t **__ppc16Write, const char     **__ppchRead ) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
