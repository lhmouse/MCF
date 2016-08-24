// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_EXT_UTF_H_
#define __MCFCRT_EXT_UTF_H_

#include "../env/_crtdef.h"

#ifndef __MCFCRT_UTF_INLINE_OR_EXTERN
#	define __MCFCRT_UTF_INLINE_OR_EXTERN     __attribute__((__gnu_inline__)) extern inline
#endif

_MCFCRT_EXTERN_C_BEGIN

// 这些函数并不对字符串结束符进行特殊处理（字符串结束符被当作普通字符）。

typedef enum __MCFCRT_tagUtf8CodeUnitType {
	_MCFCRT_kUtf8Ascii        = 1,
	_MCFCRT_kUtf8Trailing     = 2,
	_MCFCRT_kUtf8LeadingTwo   = 3,
	_MCFCRT_kUtf8LeadingThree = 4,
	_MCFCRT_kUtf8LeadingFour  = 5,
	_MCFCRT_kUtf8Reserved     = 6,
} _MCFCRT_Utf8CodeUnitType;

__attribute__((__const__))
__MCFCRT_UTF_INLINE_OR_EXTERN _MCFCRT_Utf8CodeUnitType _MCFCRT_GetUtf8CodeUnitType(char __chCodeUnit) _MCFCRT_NOEXCEPT {
	_MCFCRT_STD uint_fast32_t __u32Unit = (_MCFCRT_STD uint8_t)__chCodeUnit;
	if(__u32Unit < 0x80){
		return _MCFCRT_kUtf8Ascii;
	} else if(__u32Unit < 0xC0){
		return _MCFCRT_kUtf8Trailing;
	} else if(__u32Unit < 0xE0){
		return _MCFCRT_kUtf8LeadingTwo;
	} else if(__u32Unit < 0xF0){
		return _MCFCRT_kUtf8LeadingThree;
	} else if(__u32Unit < 0xF8){
		return _MCFCRT_kUtf8LeadingFour;
	} else {
		return _MCFCRT_kUtf8Reserved;
	}
}

typedef enum __MCFCRT_tagUtf16CodeUnitType {
	_MCFCRT_kUtf16Normal            = 1,
	_MCFCRT_kUtf16LeadingSurrogate  = 2,
	_MCFCRT_kUtf16TrailingSurrogate = 3,
} _MCFCRT_Utf16CodeUnitType;

__attribute__((__const__))
__MCFCRT_UTF_INLINE_OR_EXTERN _MCFCRT_Utf16CodeUnitType _MCFCRT_GetUtf16CodeUnitType(char16_t __c16CodeUnit) _MCFCRT_NOEXCEPT {
	_MCFCRT_STD uint_fast32_t __u32Unit = (_MCFCRT_STD uint16_t)__c16CodeUnit;
	if(__u32Unit < 0xD800){
		return _MCFCRT_kUtf16Normal;
	} else if(__u32Unit < 0xDC00){
		return _MCFCRT_kUtf16LeadingSurrogate;
	} else if(__u32Unit < 0xE000){
		return _MCFCRT_kUtf16TrailingSurrogate;
	} else {
		return _MCFCRT_kUtf16Normal;
	}
}

#define _MCFCRT_UTF_SUCCESS(__c_)     ((_MCFCRT_STD int32_t)(__c_) >= 0)

#define _MCFCRT_UTF_INVALID_INPUT     ((char32_t)-1)  // 输入字符串不是合法的 UTF 序列。
#define _MCFCRT_UTF_PARTIAL_DATA      ((char32_t)-2)  // 输入的最后一个码点不完整。
#define _MCFCRT_UTF_BUFFER_TOO_SMALL  ((char32_t)-3)  // 输出缓冲区太小。

__MCFCRT_UTF_INLINE_OR_EXTERN char32_t _MCFCRT_DecodeUtf8(const char **__ppchRead, const char *__pchReadEnd,
	bool __bIgnoreEncodingErrors) _MCFCRT_NOEXCEPT
{
	const char *__pchRead = *__ppchRead;
	if(__pchRead == __pchReadEnd){
		return _MCFCRT_UTF_PARTIAL_DATA;
	}
	const _MCFCRT_STD uint_fast32_t __u32Unit = (_MCFCRT_STD uint8_t)*(__pchRead++);
	_MCFCRT_STD uint_fast32_t __u32CodePoint;
#define __MCFCRT_HANDLE_INVALID_INPUT	\
	{	\
		if(!__bIgnoreEncodingErrors){	\
			return _MCFCRT_UTF_INVALID_INPUT;	\
		}	\
		__u32CodePoint = 0xFFFD;	\
		goto __jDone;	\
	}
#define __MCFCRT_DECODE_ONE(__bits_)	\
	{	\
		const _MCFCRT_STD uint_fast32_t __u32Next = (_MCFCRT_STD uint8_t)*(__pchRead++);	\
		if((__u32Next < 0x80) || (0xC0 <= __u32Next)){	\
			__MCFCRT_HANDLE_INVALID_INPUT	\
		}	\
		__u32CodePoint |= (__u32Next & 0x3F) << (__bits_);	\
	}
	if(__u32Unit < 0x80){
		__u32CodePoint = __u32Unit;
	} else if(__u32Unit < 0xC0){
		__MCFCRT_HANDLE_INVALID_INPUT
	} else if(__u32Unit < 0xE0){
		if(__pchReadEnd - __pchRead < 1){
			return _MCFCRT_UTF_PARTIAL_DATA;
		}
		__u32CodePoint = (__u32Unit & 0x1F) << 6;
		__MCFCRT_DECODE_ONE(0)
	} else if(__u32Unit < 0xF0){
		if(__pchReadEnd - __pchRead < 2){
			return _MCFCRT_UTF_PARTIAL_DATA;
		}
		__u32CodePoint = (__u32Unit & 0x0F) << 12;
		__MCFCRT_DECODE_ONE(6)
		__MCFCRT_DECODE_ONE(0)
		if(__u32CodePoint - 0xD800 < 0x800){
			__MCFCRT_HANDLE_INVALID_INPUT
		}
	} else if(__u32Unit < 0xF8){
		if(__pchReadEnd - __pchRead < 3){
			return _MCFCRT_UTF_PARTIAL_DATA;
		}
		__u32CodePoint = (__u32Unit & 0x07) << 18;
		__MCFCRT_DECODE_ONE(12)
		__MCFCRT_DECODE_ONE(6)
		__MCFCRT_DECODE_ONE(0)
		if(__u32CodePoint - 0xD800 < 0x800){
			__MCFCRT_HANDLE_INVALID_INPUT
		}
		if(__u32CodePoint >= 0x110000){
			__MCFCRT_HANDLE_INVALID_INPUT
		}
	} else {
		__MCFCRT_HANDLE_INVALID_INPUT
	}
#undef __MCFCRT_DECODE_ONE
#undef __MCFCRT_HANDLE_INVALID_INPUT
__jDone:
	*__ppchRead = __pchRead;
	return (char32_t)__u32CodePoint;
}
__MCFCRT_UTF_INLINE_OR_EXTERN char32_t _MCFCRT_DecodeUtf16(const char16_t **__ppc16Read, const char16_t *__pc16ReadEnd,
	bool __bIgnoreEncodingErrors) _MCFCRT_NOEXCEPT
{
	const char16_t *__pc16Read = *__ppc16Read;
	if(__pc16Read == __pc16ReadEnd){
		return _MCFCRT_UTF_PARTIAL_DATA;
	}
	const _MCFCRT_STD uint_fast32_t __u32Unit = (_MCFCRT_STD uint16_t)*(__pc16Read++);
	_MCFCRT_STD uint_fast32_t __u32CodePoint;
#define __MCFCRT_HANDLE_INVALID_INPUT	\
	{	\
		if(!__bIgnoreEncodingErrors){	\
			return _MCFCRT_UTF_INVALID_INPUT;	\
		}	\
		__u32CodePoint = 0xFFFD;	\
		goto __jDone;	\
	}
	if(__u32Unit < 0xD800){
		__u32CodePoint = __u32Unit;
	} else if(__u32Unit < 0xDC00){
		if(__pc16ReadEnd - __pc16Read < 1){
			return _MCFCRT_UTF_PARTIAL_DATA;
		}
		__u32CodePoint = (__u32Unit & 0x3FF) << 10;
		const _MCFCRT_STD uint_fast32_t __u32Next = (_MCFCRT_STD uint16_t)*(__pc16Read++);
		if((__u32Next < 0xDC00) || (0xE000 <= __u32Next)){
			__MCFCRT_HANDLE_INVALID_INPUT
		}
		__u32CodePoint |= (__u32Next & 0x3FF);
		__u32CodePoint += 0x10000;
	} else if(__u32Unit < 0xE000){
		__MCFCRT_HANDLE_INVALID_INPUT
	} else {
		__u32CodePoint = __u32Unit;
	}
#undef __MCFCRT_HANDLE_INVALID_INPUT
__jDone:
	*__ppc16Read = __pc16Read;
	return (char32_t)__u32CodePoint;
}

__MCFCRT_UTF_INLINE_OR_EXTERN char32_t _MCFCRT_EncodeUtf8(char **__ppchWrite, char *__pchWriteEnd, char32_t __c32Char,
	bool __bIgnoreEncodingErrors, bool __bEncodeNullAsTwoBytes) _MCFCRT_NOEXCEPT
{
	char *__pchWrite = *__ppchWrite;
	_MCFCRT_STD uint_fast32_t __u32CodePoint = __c32Char;
#define __MCFCRT_HANDLE_INVALID_INPUT	\
	{	\
		if(!__bIgnoreEncodingErrors){	\
			return _MCFCRT_UTF_INVALID_INPUT;	\
		}	\
		__u32CodePoint = 0xFFFD;	\
		goto __jReplace;	\
	}
	if((__u32CodePoint < 0x80) && !(__bEncodeNullAsTwoBytes && (__u32CodePoint == 0))){
		if(__pchWriteEnd - __pchWrite < 1){
			return _MCFCRT_UTF_BUFFER_TOO_SMALL;
		}
		*(__pchWrite++) = (char)__u32CodePoint;
	} else if(__u32CodePoint < 0x800){
		if(__pchWriteEnd - __pchWrite < 2){
			return _MCFCRT_UTF_BUFFER_TOO_SMALL;
		}
		*(__pchWrite++) = (char)(((__u32CodePoint >>  6) & 0x1F) | 0xC0);
		*(__pchWrite++) = (char)(((__u32CodePoint      ) & 0x3F) | 0x80);
	} else if(__u32CodePoint < 0x10000){
		if(__u32CodePoint - 0xD800 < 0x800){
			__MCFCRT_HANDLE_INVALID_INPUT
		}
__jReplace:
		if(__pchWriteEnd - __pchWrite < 3){
			return _MCFCRT_UTF_BUFFER_TOO_SMALL;
		}
		*(__pchWrite++) = (char)(((__u32CodePoint >> 12) & 0x0F) | 0xE0);
		*(__pchWrite++) = (char)(((__u32CodePoint >>  6) & 0x3F) | 0x80);
		*(__pchWrite++) = (char)(((__u32CodePoint      ) & 0x3F) | 0x80);
	} else if(__u32CodePoint < 0x110000){
		if(__pchWriteEnd - __pchWrite < 4){
			return _MCFCRT_UTF_BUFFER_TOO_SMALL;
		}
		*(__pchWrite++) = (char)(((__u32CodePoint >> 18) & 0x07) | 0xF0);
		*(__pchWrite++) = (char)(((__u32CodePoint >> 12) & 0x3F) | 0x80);
		*(__pchWrite++) = (char)(((__u32CodePoint >>  6) & 0x3F) | 0x80);
		*(__pchWrite++) = (char)(((__u32CodePoint      ) & 0x3F) | 0x80);
	} else {
		__MCFCRT_HANDLE_INVALID_INPUT
	}
#undef __MCFCRT_HANDLE_INVALID_INPUT
	*__ppchWrite = __pchWrite;
	return __u32CodePoint;
}
__MCFCRT_UTF_INLINE_OR_EXTERN char32_t _MCFCRT_EncodeUtf16(char16_t **__ppc16Write, char16_t *__pc16WriteEnd, char32_t __c32Char,
	bool __bIgnoreEncodingErrors) _MCFCRT_NOEXCEPT
{
	char16_t *__pc16Write = *__ppc16Write;
	_MCFCRT_STD uint_fast32_t __u32CodePoint = __c32Char;
#define __MCFCRT_HANDLE_INVALID_INPUT	\
	{	\
		if(!__bIgnoreEncodingErrors){	\
			return _MCFCRT_UTF_INVALID_INPUT;	\
		}	\
		__u32CodePoint = 0xFFFD;	\
		goto __jReplace;	\
	}
	if(__u32CodePoint < 0x10000){
		if(__u32CodePoint - 0xD800 < 0x800){
			__MCFCRT_HANDLE_INVALID_INPUT
		}
__jReplace:
		if(__pc16WriteEnd - __pc16Write < 1){
			return _MCFCRT_UTF_BUFFER_TOO_SMALL;
		}
		*(__pc16Write++) = (char16_t)__u32CodePoint;
	} else if(__u32CodePoint < 0x110000){
		if(__pc16WriteEnd - __pc16Write < 2){
			return _MCFCRT_UTF_BUFFER_TOO_SMALL;
		}
		*(__pc16Write++) = (char16_t)((((__u32CodePoint - 0x10000) >> 10) & 0x03FF) | 0xD800);
		*(__pc16Write++) = (char16_t)((((__u32CodePoint          )      ) & 0x03FF) | 0xDC00);
	} else {
		__MCFCRT_HANDLE_INVALID_INPUT
	}
#undef __MCFCRT_HANDLE_INVALID_INPUT
	*__ppc16Write = __pc16Write;
	return __u32CodePoint;
}

__MCFCRT_UTF_INLINE_OR_EXTERN char32_t _MCFCRT_UncheckedEncodeUtf8(char **__ppchWrite, char32_t __c32Char,
	bool __bIgnoreEncodingErrors, bool __bEncodeNullAsTwoBytes) _MCFCRT_NOEXCEPT
{
	char *__pchWrite = *__ppchWrite;
	_MCFCRT_STD uint_fast32_t __u32CodePoint = __c32Char;
#define __MCFCRT_HANDLE_INVALID_INPUT	\
	{	\
		if(!__bIgnoreEncodingErrors){	\
			return _MCFCRT_UTF_INVALID_INPUT;	\
		}	\
		__u32CodePoint = 0xFFFD;	\
		goto __jReplace;	\
	}
	if((__u32CodePoint < 0x80) && !(__bEncodeNullAsTwoBytes && (__u32CodePoint == 0))){
		*(__pchWrite++) = (char)__u32CodePoint;
	} else if(__u32CodePoint < 0x800){
		*(__pchWrite++) = (char)(((__u32CodePoint >>  6) & 0x1F) | 0xC0);
		*(__pchWrite++) = (char)(((__u32CodePoint      ) & 0x3F) | 0x80);
	} else if(__u32CodePoint < 0x10000){
		if(__u32CodePoint - 0xD800 < 0x800){
			__MCFCRT_HANDLE_INVALID_INPUT
		}
__jReplace:
		*(__pchWrite++) = (char)(((__u32CodePoint >> 12) & 0x0F) | 0xE0);
		*(__pchWrite++) = (char)(((__u32CodePoint >>  6) & 0x3F) | 0x80);
		*(__pchWrite++) = (char)(((__u32CodePoint      ) & 0x3F) | 0x80);
	} else if(__u32CodePoint < 0x110000){
		*(__pchWrite++) = (char)(((__u32CodePoint >> 18) & 0x07) | 0xF0);
		*(__pchWrite++) = (char)(((__u32CodePoint >> 12) & 0x3F) | 0x80);
		*(__pchWrite++) = (char)(((__u32CodePoint >>  6) & 0x3F) | 0x80);
		*(__pchWrite++) = (char)(((__u32CodePoint      ) & 0x3F) | 0x80);
	} else {
		__MCFCRT_HANDLE_INVALID_INPUT
	}
#undef __MCFCRT_HANDLE_INVALID_INPUT
	*__ppchWrite = __pchWrite;
	return __u32CodePoint;
}
__MCFCRT_UTF_INLINE_OR_EXTERN char32_t _MCFCRT_UncheckedEncodeUtf16(char16_t **__ppc16Write, char32_t __c32Char,
	bool __bIgnoreEncodingErrors) _MCFCRT_NOEXCEPT
{
	char16_t *__pc16Write = *__ppc16Write;
	_MCFCRT_STD uint_fast32_t __u32CodePoint = __c32Char;
#define __MCFCRT_HANDLE_INVALID_INPUT	\
	{	\
		if(!__bIgnoreEncodingErrors){	\
			return _MCFCRT_UTF_INVALID_INPUT;	\
		}	\
		__u32CodePoint = 0xFFFD;	\
		goto __jReplace;	\
	}
	if(__u32CodePoint < 0x10000){
		if(__u32CodePoint - 0xD800 < 0x800){
			__MCFCRT_HANDLE_INVALID_INPUT
		}
__jReplace:
		*(__pc16Write++) = (char16_t)__u32CodePoint;
	} else if(__u32CodePoint < 0x110000){
		*(__pc16Write++) = (char16_t)((((__u32CodePoint - 0x10000) >> 10) & 0x03FF) | 0xD800);
		*(__pc16Write++) = (char16_t)((((__u32CodePoint          )      ) & 0x03FF) | 0xDC00);
	} else {
		__MCFCRT_HANDLE_INVALID_INPUT
	}
#undef __MCFCRT_HANDLE_INVALID_INPUT
	*__ppc16Write = __pc16Write;
	return __u32CodePoint;
}

_MCFCRT_EXTERN_C_END

#endif
