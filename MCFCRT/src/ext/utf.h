// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_EXT_UTF_H_
#define __MCFCRT_EXT_UTF_H_

#include "../env/_crtdef.h"
#include "assert.h"
#include "pp.h"

#ifndef __MCFCRT_UTF_INLINE_OR_EXTERN
#	define __MCFCRT_UTF_INLINE_OR_EXTERN     __attribute__((__gnu_inline__)) extern inline
#endif

_MCFCRT_EXTERN_C_BEGIN

// 这些函数并不对字符串结束符进行特殊处理（字符串结束符被当作普通字符）。

#define _MCFCRT_UTF_SUCCESS(__c_)        ((_MCFCRT_STD int32_t)(__c_) >= 0)

#define _MCFCRT_UTF_INVALID_INPUT        ((char32_t)-1)  // 输入字符串不是合法的 UTF 序列。
#define _MCFCRT_UTF_PARTIAL_DATA         ((char32_t)-2)  // 输入的最后一个码点不完整。
#define _MCFCRT_UTF_BUFFER_TOO_SMALL     ((char32_t)-3)  // 输出缓冲区太小。

#define __MCFCRT_HANDLE_INVALID_INPUT(__permissive_, __code_point_, __goto_label_)	\
	{	\
	_MCFCRT_PP_LAZY(_MCFCRT_PP_CAT2, __MCFCRT_jCold, __LINE__): __attribute__((__cold__, __unused__));	\
		if(!(__permissive_)){	\
			return _MCFCRT_UTF_INVALID_INPUT;	\
		}	\
		(__code_point_) = 0xFFFD;	\
		goto __goto_label_;	\
	}

__MCFCRT_UTF_INLINE_OR_EXTERN char32_t _MCFCRT_DecodeUtf8(const char **__ppchRead, const char *__pchReadEnd, bool __bPermissive) _MCFCRT_NOEXCEPT {
	const char *__pchRead = *__ppchRead;
	if(__pchRead == __pchReadEnd){
		return _MCFCRT_UTF_PARTIAL_DATA;
	}
	const _MCFCRT_STD uint_fast32_t __u32Unit = (_MCFCRT_STD uint8_t)*(__pchRead++);
#define __MCFCRT_DECODE_ONE(__reg_, __bits_)	\
	{	\
		const _MCFCRT_STD uint_fast32_t __u32NextUnit_ = (_MCFCRT_STD uint8_t)*(__pchRead++);	\
		_MCFCRT_STD uint_fast32_t __u32Test_;	\
		if((__u32Test_ = __u32NextUnit_ - 0x80) >= 0x40){	\
			__MCFCRT_HANDLE_INVALID_INPUT(__bPermissive, (__reg_), __jDone)	\
		}	\
		(__reg_) += __u32Test_ << (__bits_);	\
	}
	_MCFCRT_STD uint_fast32_t __u32CodePoint, __u32Test;
	if(__u32Unit < 0x80){
		__u32CodePoint = __u32Unit;
	} else if((__u32Test = __u32Unit - 0x80) < 0x40){
		__MCFCRT_HANDLE_INVALID_INPUT(__bPermissive, __u32CodePoint, __jDone)
	} else if((__u32Test = __u32Unit - 0xC0) < 0x20){
		if(__pchReadEnd - __pchRead < 1){
			return _MCFCRT_UTF_PARTIAL_DATA;
		}
		__u32CodePoint = __u32Test << 6;
		__MCFCRT_DECODE_ONE(__u32CodePoint, 0)
		if(__u32CodePoint < 0x80){
			__MCFCRT_HANDLE_INVALID_INPUT(__bPermissive, __u32CodePoint, __jDone)
		}
	} else if((__u32Test = __u32Unit - 0xE0) < 0x10){
		if(__pchReadEnd - __pchRead < 2){
			return _MCFCRT_UTF_PARTIAL_DATA;
		}
		__u32CodePoint = __u32Test << 12;
		__MCFCRT_DECODE_ONE(__u32CodePoint, 6)
		__MCFCRT_DECODE_ONE(__u32CodePoint, 0)
		if(__u32CodePoint < 0x800){
			__MCFCRT_HANDLE_INVALID_INPUT(__bPermissive, __u32CodePoint, __jDone)
		}
		if(__u32CodePoint - 0xD800 < 0x800){
			__MCFCRT_HANDLE_INVALID_INPUT(__bPermissive, __u32CodePoint, __jDone)
		}
	} else if((__u32Test = __u32Unit - 0xF0) < 0x08){
		if(__pchReadEnd - __pchRead < 3){
			return _MCFCRT_UTF_PARTIAL_DATA;
		}
		__u32CodePoint = __u32Test << 18;
		__MCFCRT_DECODE_ONE(__u32CodePoint, 12)
		__MCFCRT_DECODE_ONE(__u32CodePoint,  6)
		__MCFCRT_DECODE_ONE(__u32CodePoint,  0)
		if(__u32CodePoint < 0x10000){
			__MCFCRT_HANDLE_INVALID_INPUT(__bPermissive, __u32CodePoint, __jDone)
		}
		if(__u32CodePoint >= 0x110000){
			__MCFCRT_HANDLE_INVALID_INPUT(__bPermissive, __u32CodePoint, __jDone)
		}
	} else {
		__MCFCRT_HANDLE_INVALID_INPUT(__bPermissive, __u32CodePoint, __jDone)
	}
#undef __MCFCRT_DECODE_ONE
__jDone:
	*__ppchRead = __pchRead;
	_MCFCRT_ASSERT((__u32CodePoint < 0x110000) && (__u32CodePoint - 0xD800 >= 0x800));
	return (char32_t)__u32CodePoint;
}
__MCFCRT_UTF_INLINE_OR_EXTERN char32_t _MCFCRT_DecodeUtf16(const char16_t **__ppc16Read, const char16_t *__pc16ReadEnd, bool __bPermissive) _MCFCRT_NOEXCEPT {
	const char16_t *__pc16Read = *__ppc16Read;
	if(__pc16Read == __pc16ReadEnd){
		return _MCFCRT_UTF_PARTIAL_DATA;
	}
	const _MCFCRT_STD uint_fast32_t __u32Unit = (_MCFCRT_STD uint16_t)*(__pc16Read++);
	_MCFCRT_STD uint_fast32_t __u32CodePoint, __u32Test;
	if((__u32Test = __u32Unit - 0xD800) < 0x400){
		if(__pc16ReadEnd - __pc16Read < 1){
			return _MCFCRT_UTF_PARTIAL_DATA;
		}
		__u32CodePoint = __u32Test << 10;
		_MCFCRT_STD uint_fast32_t __u32NextUnit = (_MCFCRT_STD uint16_t)*(__pc16Read++);
		if((__u32Test = __u32NextUnit - 0xDC00) >= 0x400){
			__MCFCRT_HANDLE_INVALID_INPUT(__bPermissive, __u32CodePoint, __jDone)
		}
		__u32CodePoint += __u32Test;
		__u32CodePoint += 0x10000;
	} else if(__u32Test < 0x800){
		__MCFCRT_HANDLE_INVALID_INPUT(__bPermissive, __u32CodePoint, __jDone)
	} else {
		__u32CodePoint = __u32Unit;
	}
__jDone:
	*__ppc16Read = __pc16Read;
	_MCFCRT_ASSERT((__u32CodePoint < 0x110000) && (__u32CodePoint - 0xD800 >= 0x800));
	return (char32_t)__u32CodePoint;
}
__MCFCRT_UTF_INLINE_OR_EXTERN char32_t _MCFCRT_DecodeUtf32(const char32_t **__ppc32Read, const char32_t *__pc32ReadEnd, bool __bPermissive) _MCFCRT_NOEXCEPT {
	const char32_t *__pc32Read = *__ppc32Read;
	if(__pc32Read == __pc32ReadEnd){
		return _MCFCRT_UTF_PARTIAL_DATA;
	}
	const _MCFCRT_STD uint_fast32_t __u32Unit = (_MCFCRT_STD uint32_t)*(__pc32Read++);
	_MCFCRT_STD uint_fast32_t __u32CodePoint;
	if(__u32Unit - 0xD800 < 0x800){
		__MCFCRT_HANDLE_INVALID_INPUT(__bPermissive, __u32CodePoint, __jDone)
	} else if(__u32Unit < 0x110000){
		__u32CodePoint = __u32Unit;
	} else {
		__MCFCRT_HANDLE_INVALID_INPUT(__bPermissive, __u32CodePoint, __jDone)
	}
__jDone:
	*__ppc32Read = __pc32Read;
	_MCFCRT_ASSERT((__u32CodePoint < 0x110000) && (__u32CodePoint - 0xD800 >= 0x800));
	return (char32_t)__u32CodePoint;
}
__MCFCRT_UTF_INLINE_OR_EXTERN char32_t _MCFCRT_DecodeCesu8(const char **__ppchRead, const char *__pchReadEnd, bool __bPermissive) _MCFCRT_NOEXCEPT {
	const char *__pchRead = *__ppchRead;
	if(__pchRead == __pchReadEnd){
		return _MCFCRT_UTF_PARTIAL_DATA;
	}
	const _MCFCRT_STD uint_fast32_t __u32Unit = (_MCFCRT_STD uint8_t)*(__pchRead++);
#define __MCFCRT_DECODE_ONE(__reg_, __bits_)	\
	{	\
		const _MCFCRT_STD uint_fast32_t __u32NextUnit_ = (_MCFCRT_STD uint8_t)*(__pchRead++);	\
		_MCFCRT_STD uint_fast32_t __u32Test_;	\
		if((__u32Test_ = __u32NextUnit_ - 0x80) >= 0x40){	\
			__MCFCRT_HANDLE_INVALID_INPUT(__bPermissive, (__reg_), __jDone)	\
		}	\
		(__reg_) += __u32Test_ << (__bits_);	\
	}
	_MCFCRT_STD uint_fast32_t __u32CodePoint, __u32Test;
	if(__u32Unit < 0x80){
		__u32CodePoint = __u32Unit;
	} else if((__u32Test = __u32Unit - 0x80) < 0x40){
		__MCFCRT_HANDLE_INVALID_INPUT(__bPermissive, __u32CodePoint, __jDone)
	} else if((__u32Test = __u32Unit - 0xC0) < 0x20){
		if(__pchReadEnd - __pchRead < 1){
			return _MCFCRT_UTF_PARTIAL_DATA;
		}
		__u32CodePoint = __u32Test << 6;
		__MCFCRT_DECODE_ONE(__u32CodePoint, 0)
		if(__u32CodePoint < 0x80){
			__MCFCRT_HANDLE_INVALID_INPUT(__bPermissive, __u32CodePoint, __jDone)
		}
	} else if((__u32Test = __u32Unit - 0xE0) < 0x10){
		if(__pchReadEnd - __pchRead < 2){
			return _MCFCRT_UTF_PARTIAL_DATA;
		}
		__u32CodePoint = __u32Test << 12;
		__MCFCRT_DECODE_ONE(__u32CodePoint, 6)
		__MCFCRT_DECODE_ONE(__u32CodePoint, 0)
		if(__u32CodePoint < 0x800){
			__MCFCRT_HANDLE_INVALID_INPUT(__bPermissive, __u32CodePoint, __jDone)
		}
		if((__u32Test = __u32CodePoint - 0xD800) < 0x800){
			if(__u32Test >= 0x400){
				__MCFCRT_HANDLE_INVALID_INPUT(__bPermissive, __u32CodePoint, __jDone)
			}
			if(__pchReadEnd - __pchRead < 3){
				return _MCFCRT_UTF_PARTIAL_DATA;
			}
			__u32CodePoint = __u32Test << 10;
			_MCFCRT_STD uint_fast32_t __u32NextUnit = (_MCFCRT_STD uint8_t)*(__pchRead++);
			if((__u32Test = __u32NextUnit - 0xE0) >= 0x10){
				__MCFCRT_HANDLE_INVALID_INPUT(__bPermissive, __u32CodePoint, __jDone)
			}
			__u32NextUnit = __u32Test << 12;
			__MCFCRT_DECODE_ONE(__u32NextUnit, 6)
			__MCFCRT_DECODE_ONE(__u32NextUnit, 0)
			if((__u32Test = __u32NextUnit - 0xDC00) >= 0x400){
				__MCFCRT_HANDLE_INVALID_INPUT(__bPermissive, __u32CodePoint, __jDone)
			}
			__u32CodePoint += __u32Test;
			__u32CodePoint += 0x10000;
		}
	} else {
		__MCFCRT_HANDLE_INVALID_INPUT(__bPermissive, __u32CodePoint, __jDone)
	}
#undef __MCFCRT_DECODE_ONE
__jDone:
	*__ppchRead = __pchRead;
	_MCFCRT_ASSERT((__u32CodePoint < 0x110000) && (__u32CodePoint - 0xD800 >= 0x800));
	return (char32_t)__u32CodePoint;
}

__MCFCRT_UTF_INLINE_OR_EXTERN char32_t _MCFCRT_EncodeUtf8(char **__ppchWrite, char *__pchWriteEnd, char32_t __c32Char, bool __bPermissive) _MCFCRT_NOEXCEPT {
	char *__pchWrite = *__ppchWrite;
	_MCFCRT_STD uint_fast32_t __u32CodePoint = __c32Char;
	if(__u32CodePoint < 0x80){
		if(__pchWriteEnd - __pchWrite < 1){
			return _MCFCRT_UTF_BUFFER_TOO_SMALL;
		}
		*(__pchWrite++) = (char)__u32CodePoint;
	} else if(__u32CodePoint < 0x800){
		if(__pchWriteEnd - __pchWrite < 2){
			return _MCFCRT_UTF_BUFFER_TOO_SMALL;
		}
		*(__pchWrite++) = (char)(((__u32CodePoint      ) >>  6) + 0xC0);
		*(__pchWrite++) = (char)(((__u32CodePoint << 26) >> 26) + 0x80);
	} else if(__u32CodePoint < 0x10000){
		if(__u32CodePoint - 0xD800 < 0x800){
			__MCFCRT_HANDLE_INVALID_INPUT(__bPermissive, __u32CodePoint, __jReplace)
		}
__jReplace:
		if(__pchWriteEnd - __pchWrite < 3){
			return _MCFCRT_UTF_BUFFER_TOO_SMALL;
		}
		*(__pchWrite++) = (char)(((__u32CodePoint      ) >> 12) + 0xE0);
		*(__pchWrite++) = (char)(((__u32CodePoint << 20) >> 26) + 0x80);
		*(__pchWrite++) = (char)(((__u32CodePoint << 26) >> 26) + 0x80);
	} else if(__u32CodePoint < 0x110000){
		if(__pchWriteEnd - __pchWrite < 4){
			return _MCFCRT_UTF_BUFFER_TOO_SMALL;
		}
		*(__pchWrite++) = (char)(((__u32CodePoint      ) >> 18) + 0xF0);
		*(__pchWrite++) = (char)(((__u32CodePoint << 14) >> 26) + 0x80);
		*(__pchWrite++) = (char)(((__u32CodePoint << 20) >> 26) + 0x80);
		*(__pchWrite++) = (char)(((__u32CodePoint << 26) >> 26) + 0x80);
	} else {
		__MCFCRT_HANDLE_INVALID_INPUT(__bPermissive, __u32CodePoint, __jReplace)
	}
	*__ppchWrite = __pchWrite;
	_MCFCRT_ASSERT((__u32CodePoint < 0x110000) && (__u32CodePoint - 0xD800 >= 0x800));
	return __u32CodePoint;
}
__MCFCRT_UTF_INLINE_OR_EXTERN char32_t _MCFCRT_EncodeUtf16(char16_t **__ppc16Write, char16_t *__pc16WriteEnd, char32_t __c32Char, bool __bPermissive) _MCFCRT_NOEXCEPT {
	char16_t *__pc16Write = *__ppc16Write;
	_MCFCRT_STD uint_fast32_t __u32CodePoint = __c32Char;
	if(__u32CodePoint < 0x10000){
		if(__u32CodePoint - 0xD800 < 0x800){
			__MCFCRT_HANDLE_INVALID_INPUT(__bPermissive, __u32CodePoint, __jReplace)
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
		const _MCFCRT_STD uint_fast32_t __u32LeadingSurrogate  = (((__u32CodePoint - 0x10000)      ) >> 10) + 0xD800;
		const _MCFCRT_STD uint_fast32_t __u32TrailingSurrogate = (((__u32CodePoint          ) << 22) >> 22) + 0xDC00;
		*(__pc16Write++) = (char16_t)__u32LeadingSurrogate;
		*(__pc16Write++) = (char16_t)__u32TrailingSurrogate;
	} else {
		__MCFCRT_HANDLE_INVALID_INPUT(__bPermissive, __u32CodePoint, __jReplace)
	}
	*__ppc16Write = __pc16Write;
	_MCFCRT_ASSERT((__u32CodePoint < 0x110000) && (__u32CodePoint - 0xD800 >= 0x800));
	return __u32CodePoint;
}
__MCFCRT_UTF_INLINE_OR_EXTERN char32_t _MCFCRT_EncodeUtf32(char32_t **__ppc32Write, char32_t *__pc32WriteEnd, char32_t __c32Char, bool __bPermissive) _MCFCRT_NOEXCEPT {
	char32_t *__pc32Write = *__ppc32Write;
	_MCFCRT_STD uint_fast32_t __u32CodePoint = __c32Char;
	if(__u32CodePoint < 0x110000){
		if(__u32CodePoint - 0xD800 < 0x800){
			__MCFCRT_HANDLE_INVALID_INPUT(__bPermissive, __u32CodePoint, __jReplace)
		}
__jReplace:
		if(__pc32WriteEnd - __pc32Write < 1){
			return _MCFCRT_UTF_BUFFER_TOO_SMALL;
		}
		*(__pc32Write++) = (char32_t)__u32CodePoint;
	} else {
		__MCFCRT_HANDLE_INVALID_INPUT(__bPermissive, __u32CodePoint, __jReplace)
	}
	*__ppc32Write = __pc32Write;
	_MCFCRT_ASSERT((__u32CodePoint < 0x110000) && (__u32CodePoint - 0xD800 >= 0x800));
	return __u32CodePoint;
}
__MCFCRT_UTF_INLINE_OR_EXTERN char32_t _MCFCRT_EncodeCesu8(char **__ppchWrite, char *__pchWriteEnd, char32_t __c32Char, bool __bPermissive) _MCFCRT_NOEXCEPT {
	char *__pchWrite = *__ppchWrite;
	_MCFCRT_STD uint_fast32_t __u32CodePoint = __c32Char;
	if(__u32CodePoint < 0x80){
		if(__pchWriteEnd - __pchWrite < 1){
			return _MCFCRT_UTF_BUFFER_TOO_SMALL;
		}
		*(__pchWrite++) = (char)__u32CodePoint;
	} else if(__u32CodePoint < 0x800){
		if(__pchWriteEnd - __pchWrite < 2){
			return _MCFCRT_UTF_BUFFER_TOO_SMALL;
		}
		*(__pchWrite++) = (char)(((__u32CodePoint      ) >>  6) + 0xC0);
		*(__pchWrite++) = (char)(((__u32CodePoint << 26) >> 26) + 0x80);
	} else if(__u32CodePoint < 0x10000){
		if(__u32CodePoint - 0xD800 < 0x800){
			__MCFCRT_HANDLE_INVALID_INPUT(__bPermissive, __u32CodePoint,__jReplace)
		}
__jReplace:
		if(__pchWriteEnd - __pchWrite < 3){
			return _MCFCRT_UTF_BUFFER_TOO_SMALL;
		}
		*(__pchWrite++) = (char)(((__u32CodePoint      ) >> 12) + 0xE0);
		*(__pchWrite++) = (char)(((__u32CodePoint << 20) >> 26) + 0x80);
		*(__pchWrite++) = (char)(((__u32CodePoint << 26) >> 26) + 0x80);
	} else if(__u32CodePoint < 0x110000){
		if(__pchWriteEnd - __pchWrite < 6){
			return _MCFCRT_UTF_BUFFER_TOO_SMALL;
		}
		const _MCFCRT_STD uint_fast32_t __u32LeadingSurrogate  = (((__u32CodePoint - 0x10000)      ) >> 10) + 0xD800;
		const _MCFCRT_STD uint_fast32_t __u32TrailingSurrogate = (((__u32CodePoint          ) << 22) >> 22) + 0xDC00;
		*(__pchWrite++) = (char)(((__u32LeadingSurrogate       ) >> 12) + 0xE0);
		*(__pchWrite++) = (char)(((__u32LeadingSurrogate  << 20) >> 26) + 0x80);
		*(__pchWrite++) = (char)(((__u32LeadingSurrogate  << 26) >> 26) + 0x80);
		*(__pchWrite++) = (char)(((__u32TrailingSurrogate      ) >> 12) + 0xE0);
		*(__pchWrite++) = (char)(((__u32TrailingSurrogate << 20) >> 26) + 0x80);
		*(__pchWrite++) = (char)(((__u32TrailingSurrogate << 26) >> 26) + 0x80);
	} else {
		__MCFCRT_HANDLE_INVALID_INPUT(__bPermissive, __u32CodePoint, __jReplace)
	}
	*__ppchWrite = __pchWrite;
	_MCFCRT_ASSERT((__u32CodePoint < 0x110000) && (__u32CodePoint - 0xD800 >= 0x800));
	return __u32CodePoint;
}

__MCFCRT_UTF_INLINE_OR_EXTERN char32_t _MCFCRT_UncheckedEncodeUtf8(char **__ppchWrite, char32_t __c32Char, bool __bPermissive) _MCFCRT_NOEXCEPT {
	char *__pchWrite = *__ppchWrite;
	_MCFCRT_STD uint_fast32_t __u32CodePoint = __c32Char;
	if(__u32CodePoint < 0x80){
		*(__pchWrite++) = (char)__u32CodePoint;
	} else if(__u32CodePoint < 0x800){
		*(__pchWrite++) = (char)(((__u32CodePoint      ) >>  6) + 0xC0);
		*(__pchWrite++) = (char)(((__u32CodePoint << 26) >> 26) + 0x80);
	} else if(__u32CodePoint < 0x10000){
		if(__u32CodePoint - 0xD800 < 0x800){
			__MCFCRT_HANDLE_INVALID_INPUT(__bPermissive, __u32CodePoint, __jReplace)
		}
__jReplace:
		*(__pchWrite++) = (char)(((__u32CodePoint      ) >> 12) + 0xE0);
		*(__pchWrite++) = (char)(((__u32CodePoint << 20) >> 26) + 0x80);
		*(__pchWrite++) = (char)(((__u32CodePoint << 26) >> 26) + 0x80);
	} else if(__u32CodePoint < 0x110000){
		*(__pchWrite++) = (char)(((__u32CodePoint      ) >> 18) + 0xF0);
		*(__pchWrite++) = (char)(((__u32CodePoint << 14) >> 26) + 0x80);
		*(__pchWrite++) = (char)(((__u32CodePoint << 20) >> 26) + 0x80);
		*(__pchWrite++) = (char)(((__u32CodePoint << 26) >> 26) + 0x80);
	} else {
		__MCFCRT_HANDLE_INVALID_INPUT(__bPermissive, __u32CodePoint, __jReplace)
	}
	*__ppchWrite = __pchWrite;
	_MCFCRT_ASSERT((__u32CodePoint < 0x110000) && (__u32CodePoint - 0xD800 >= 0x800));
	return __u32CodePoint;
}
__MCFCRT_UTF_INLINE_OR_EXTERN char32_t _MCFCRT_UncheckedEncodeUtf16(char16_t **__ppc16Write, char32_t __c32Char, bool __bPermissive) _MCFCRT_NOEXCEPT {
	char16_t *__pc16Write = *__ppc16Write;
	_MCFCRT_STD uint_fast32_t __u32CodePoint = __c32Char;
	if(__u32CodePoint < 0x10000){
		if(__u32CodePoint - 0xD800 < 0x800){
			__MCFCRT_HANDLE_INVALID_INPUT(__bPermissive, __u32CodePoint, __jReplace)
		}
__jReplace:
		*(__pc16Write++) = (char16_t)__u32CodePoint;
	} else if(__u32CodePoint < 0x110000){
		const _MCFCRT_STD uint_fast32_t __u32LeadingSurrogate  = (((__u32CodePoint - 0x10000)      ) >> 10) + 0xD800;
		const _MCFCRT_STD uint_fast32_t __u32TrailingSurrogate = (((__u32CodePoint          ) << 22) >> 22) + 0xDC00;
		*(__pc16Write++) = (char16_t)__u32LeadingSurrogate;
		*(__pc16Write++) = (char16_t)__u32TrailingSurrogate;
	} else {
		__MCFCRT_HANDLE_INVALID_INPUT(__bPermissive, __u32CodePoint, __jReplace)
	}
	*__ppc16Write = __pc16Write;
	_MCFCRT_ASSERT((__u32CodePoint < 0x110000) && (__u32CodePoint - 0xD800 >= 0x800));
	return __u32CodePoint;
}
__MCFCRT_UTF_INLINE_OR_EXTERN char32_t _MCFCRT_UncheckedEncodeUtf32(char32_t **__ppc32Write, char32_t __c32Char, bool __bPermissive) _MCFCRT_NOEXCEPT {
	char32_t *__pc32Write = *__ppc32Write;
	_MCFCRT_STD uint_fast32_t __u32CodePoint = __c32Char;
	if(__u32CodePoint < 0x110000){
		if(__u32CodePoint - 0xD800 < 0x800){
			__MCFCRT_HANDLE_INVALID_INPUT(__bPermissive, __u32CodePoint, __jReplace)
		}
__jReplace:
		*(__pc32Write++) = (char32_t)__u32CodePoint;
	} else {
		__MCFCRT_HANDLE_INVALID_INPUT(__bPermissive, __u32CodePoint, __jReplace)
	}
	*__ppc32Write = __pc32Write;
	_MCFCRT_ASSERT((__u32CodePoint < 0x110000) && (__u32CodePoint - 0xD800 >= 0x800));
	return __u32CodePoint;
}
__MCFCRT_UTF_INLINE_OR_EXTERN char32_t _MCFCRT_UncheckedEncodeCesu8(char **__ppchWrite, char32_t __c32Char, bool __bPermissive) _MCFCRT_NOEXCEPT {
	char *__pchWrite = *__ppchWrite;
	_MCFCRT_STD uint_fast32_t __u32CodePoint = __c32Char;
	if(__u32CodePoint < 0x80){
		*(__pchWrite++) = (char)__u32CodePoint;
	} else if(__u32CodePoint < 0x800){
		*(__pchWrite++) = (char)(((__u32CodePoint      ) >>  6) + 0xC0);
		*(__pchWrite++) = (char)(((__u32CodePoint << 26) >> 26) + 0x80);
	} else if(__u32CodePoint < 0x10000){
		if(__u32CodePoint - 0xD800 < 0x800){
			__MCFCRT_HANDLE_INVALID_INPUT(__bPermissive, __u32CodePoint, __jReplace)
		}
__jReplace:
		*(__pchWrite++) = (char)(((__u32CodePoint      ) >> 12) + 0xE0);
		*(__pchWrite++) = (char)(((__u32CodePoint << 20) >> 26) + 0x80);
		*(__pchWrite++) = (char)(((__u32CodePoint << 26) >> 26) + 0x80);
	} else if(__u32CodePoint < 0x110000){
		const _MCFCRT_STD uint_fast32_t __u32LeadingSurrogate  = (((__u32CodePoint - 0x10000)      ) >> 10) + 0xD800;
		const _MCFCRT_STD uint_fast32_t __u32TrailingSurrogate = (((__u32CodePoint          ) << 22) >> 22) + 0xDC00;
		*(__pchWrite++) = (char)(((__u32LeadingSurrogate       ) >> 12) + 0xE0);
		*(__pchWrite++) = (char)(((__u32LeadingSurrogate  << 20) >> 26) + 0x80);
		*(__pchWrite++) = (char)(((__u32LeadingSurrogate  << 26) >> 26) + 0x80);
		*(__pchWrite++) = (char)(((__u32TrailingSurrogate      ) >> 12) + 0xE0);
		*(__pchWrite++) = (char)(((__u32TrailingSurrogate << 20) >> 26) + 0x80);
		*(__pchWrite++) = (char)(((__u32TrailingSurrogate << 26) >> 26) + 0x80);
	} else {
		__MCFCRT_HANDLE_INVALID_INPUT(__bPermissive, __u32CodePoint, __jReplace)
	}
	*__ppchWrite = __pchWrite;
	_MCFCRT_ASSERT((__u32CodePoint < 0x110000) && (__u32CodePoint - 0xD800 >= 0x800));
	return __u32CodePoint;
}

#undef __MCFCRT_HANDLE_INVALID_INPUT

_MCFCRT_EXTERN_C_END

#endif
