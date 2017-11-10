// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_EXT_MUL64_H_
#define __MCFCRT_EXT_MUL64_H_

#include "../env/_crtdef.h"

#ifndef __MCFCRT_MUL64_INLINE_OR_EXTERN
#  define __MCFCRT_MUL64_INLINE_OR_EXTERN     __attribute__((__gnu_inline__)) extern inline
#endif

_MCFCRT_EXTERN_C_BEGIN

__MCFCRT_MUL64_INLINE_OR_EXTERN _MCFCRT_STD int64_t __MCFCRT_imul64_nonconstexpr(_MCFCRT_STD int32_t __a, _MCFCRT_STD int32_t __b) _MCFCRT_NOEXCEPT {
	_MCFCRT_STD int64_t __product;
	__asm__(
#ifdef _WIN64
		"imul %2 \n"
		: "=a"(__product)
		: "a"((_MCFCRT_STD int64_t)__a), "rm"((_MCFCRT_STD int64_t)__b)
#else
		"imul %2 \n"
		: "=A"(__product)
		: "a"((_MCFCRT_STD int32_t)__a), "rm"((_MCFCRT_STD int32_t)__b)
#endif
	);
	return __product;
}

_MCFCRT_CONSTEXPR __MCFCRT_MUL64_INLINE_OR_EXTERN _MCFCRT_STD int64_t _MCFCRT_imul64(_MCFCRT_STD int32_t __a, _MCFCRT_STD int32_t __b) _MCFCRT_NOEXCEPT {
	return __builtin_constant_p(__a * __b) ? (__a * (_MCFCRT_STD int64_t)__b)
	                                       : __MCFCRT_imul64_nonconstexpr(__a, __b);
}

__MCFCRT_MUL64_INLINE_OR_EXTERN _MCFCRT_STD uint64_t __MCFCRT_umul64_nonconstexpr(_MCFCRT_STD uint32_t __a, _MCFCRT_STD uint32_t __b) _MCFCRT_NOEXCEPT {
	_MCFCRT_STD uint64_t __product;
	__asm__(
#ifdef _WIN64
		"mul %2 \n"
		: "=a"(__product)
		: "a"((_MCFCRT_STD uint64_t)__a), "rm"((_MCFCRT_STD uint64_t)__b)
#else
		"mul %2 \n"
		: "=A"(__product)
		: "a"((_MCFCRT_STD uint32_t)__a), "rm"((_MCFCRT_STD uint32_t)__b)
#endif
	);
	return __product;
}

_MCFCRT_CONSTEXPR __MCFCRT_MUL64_INLINE_OR_EXTERN _MCFCRT_STD uint64_t _MCFCRT_umul64(_MCFCRT_STD uint32_t __a, _MCFCRT_STD uint32_t __b) _MCFCRT_NOEXCEPT {
	return __builtin_constant_p(__a * __b) ? (__a * (_MCFCRT_STD uint64_t)__b)
	                                       : __MCFCRT_umul64_nonconstexpr(__a, __b);
}

_MCFCRT_EXTERN_C_END

#endif
