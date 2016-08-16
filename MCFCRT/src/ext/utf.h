// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_EXT_UTF_H_
#define __MCFCRT_EXT_UTF_H_

#include "../env/_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

// 这些函数并不对字符串结束符进行特殊处理（字符串结束符被当作普通字符）。

typedef enum __MCFCRT_tagUtf8CodeUnitType {
	_MCFCRT_kUtf8Trailing     = 0,
	_MCFCRT_kUtf8Ascii        = 1,
	_MCFCRT_kUtf8LeadingTwo   = 2,
	_MCFCRT_kUtf8LeadingThree = 3,
	_MCFCRT_kUtf8LeadingFour  = 4,
	_MCFCRT_kUtf8Reserved     = 5,
} _MCFCRT_Utf8CodeUnitType;

__attribute__((__const__))
extern _MCFCRT_Utf8CodeUnitType _MCFCRT_GetUtf8CodeUnitType(char __chCodeUnit) _MCFCRT_NOEXCEPT;

typedef enum __MCFCRT_tagUtf16CodeUnitType {
	_MCFCRT_kUtf16Normal            = 0,
	_MCFCRT_kUtf16LeadingSurrogate  = 1,
	_MCFCRT_kUtf16TrailingSurrogate = 2,
} _MCFCRT_Utf16CodeUnitType;

__attribute__((__const__))
extern _MCFCRT_Utf16CodeUnitType _MCFCRT_GetUtf16CodeUnitType(char16_t __c16CodeUnit) _MCFCRT_NOEXCEPT;

#define _MCFCRT_UTF_SUCCESS(__c_)     ((_MCFCRT_STD int32_t)(__c_) >= 0)

#define _MCFCRT_UTF_INVALID_INPUT     ((char32_t)-1)  // 输入字符串不是合法的 UTF 序列。
#define _MCFCRT_UTF_PARTIAL_DATA      ((char32_t)-2)  // 输入的最后一个码点不完整。
#define _MCFCRT_UTF_BUFFER_TOO_SMALL  ((char32_t)-3)  // 输出缓冲区太小。

// 带错误检测的版本。
// 使用 _MCFCRT_UTF_SUCCESS() 判断是否成功。如果这些函数成功，输入的指针将指向上一次读取或写入的末尾；否则，输入的指针被原封不动保留。
extern char32_t _MCFCRT_DecodeUtf8 (const char     **__ppchRead , const char     *__pchReadEnd ) _MCFCRT_NOEXCEPT;
extern char32_t _MCFCRT_DecodeUtf16(const char16_t **__ppc16Read, const char16_t *__pc16ReadEnd) _MCFCRT_NOEXCEPT;

extern char32_t _MCFCRT_EncodeUtf8 (char     **__ppchWrite , char     *__pchWriteEnd , char32_t __c32CodePoint) _MCFCRT_NOEXCEPT;
extern char32_t _MCFCRT_EncodeUtf16(char16_t **__ppc16Write, char16_t *__pc16WriteEnd, char32_t __c32CodePoint) _MCFCRT_NOEXCEPT;

extern char32_t _MCFCRT_EncodeUtf8FromUtf16(char     **__ppchWrite , char     *__pchWriteEnd , const char16_t **__ppc16Read, const char16_t *__pc16ReadEnd) _MCFCRT_NOEXCEPT;
extern char32_t _MCFCRT_EncodeUtf16FromUtf8(char16_t **__ppc16Write, char16_t *__pc16WriteEnd, const char     **__ppchRead , const char     *__pchReadEnd ) _MCFCRT_NOEXCEPT;

// 不带错误检测的版本。这些函数以运行效率为首要目标。
// 为了防止访问越界，输入输出的缓冲区应当能够容纳合法的最大编码点。对于 UTF-8 而言这个值是 4 个编码单元（char），而对于 UTF-16 这个值是 2 个编码单元（char16_t）。
extern char32_t _MCFCRT_UncheckedDecodeUtf8 (const char     *restrict *restrict __ppchRead ) _MCFCRT_NOEXCEPT;
extern char32_t _MCFCRT_UncheckedDecodeUtf16(const char16_t *restrict *restrict __ppc16Read) _MCFCRT_NOEXCEPT;

extern char32_t _MCFCRT_UncheckedEncodeUtf8 (char     *restrict *restrict __ppchWrite , char32_t __c32CodePoint) _MCFCRT_NOEXCEPT;
extern char32_t _MCFCRT_UncheckedEncodeUtf16(char16_t *restrict *restrict __ppc16Write, char32_t __c32CodePoint) _MCFCRT_NOEXCEPT;

extern char32_t _MCFCRT_UncheckedEncodeUtf8FromUtf16(char     *restrict *restrict __ppchWrite , const char16_t *restrict *restrict __ppc16Read) _MCFCRT_NOEXCEPT;
extern char32_t _MCFCRT_UncheckedEncodeUtf16FromUtf8(char16_t *restrict *restrict __ppc16Write, const char     *restrict *restrict __ppchRead ) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
