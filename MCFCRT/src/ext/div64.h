// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_EXT_DIV64_H_
#define __MCFCRT_EXT_DIV64_H_

#include "../env/_crtdef.h"

#ifndef __MCFCRT_DIV64_INLINE_OR_EXTERN
#  define __MCFCRT_DIV64_INLINE_OR_EXTERN     __attribute__((__gnu_inline__)) extern inline
#endif

_MCFCRT_EXTERN_C_BEGIN

__MCFCRT_DIV64_INLINE_OR_EXTERN _MCFCRT_STD int32_t __MCFCRT_idiv64_nonconstexpr(_MCFCRT_STD int32_t *_MCFCRT_RESTRICT __rem_ret, _MCFCRT_STD int64_t __a, _MCFCRT_STD int32_t __b) _MCFCRT_NOEXCEPT {
	_MCFCRT_STD int32_t __quo, __rem;
	__asm__ (
#ifdef _WIN64
		"cqo \n"
		"idiv %3 \n"
		: "=a"(__quo), "=d"(__rem)
		: "a"(__a), "rm"((_MCFCRT_STD int64_t)__b)
#else
		"idiv %3 \n"
		: "=a"(__quo), "=d"(__rem)
		: "A"(__a), "rm"(__b)
#endif
	);
	if(__rem_ret){
		*__rem_ret = __rem;
	}
	return __quo;
}

_MCFCRT_CONSTEXPR __MCFCRT_DIV64_INLINE_OR_EXTERN _MCFCRT_STD int32_t _MCFCRT_idiv64(_MCFCRT_STD int64_t __a, _MCFCRT_STD int32_t __b) _MCFCRT_NOEXCEPT {
	return __builtin_constant_p(__a / __b) ? (_MCFCRT_STD int32_t)(__a / (_MCFCRT_STD int64_t)__b)
	                                       : __MCFCRT_idiv64_nonconstexpr(_MCFCRT_NULLPTR, __a, __b);
}
__MCFCRT_DIV64_INLINE_OR_EXTERN _MCFCRT_STD int32_t _MCFCRT_idivrem64(_MCFCRT_STD int32_t *_MCFCRT_RESTRICT __rem_ret, _MCFCRT_STD int64_t __a, _MCFCRT_STD int32_t __b) _MCFCRT_NOEXCEPT {
	return __builtin_constant_p(__a / __b) ? ((__rem_ret ? (*__rem_ret = (_MCFCRT_STD int32_t)(__a % (_MCFCRT_STD int64_t)__b)) : 0), (_MCFCRT_STD int32_t)(__a / (_MCFCRT_STD int64_t)__b))
	                                       : __MCFCRT_idiv64_nonconstexpr(__rem_ret, __a, __b);
}

__MCFCRT_DIV64_INLINE_OR_EXTERN _MCFCRT_STD uint32_t __MCFCRT_udiv64_nonconstexpr(_MCFCRT_STD uint32_t *_MCFCRT_RESTRICT __rem_ret, _MCFCRT_STD uint64_t __a, _MCFCRT_STD uint32_t __b) _MCFCRT_NOEXCEPT {
	_MCFCRT_STD uint32_t __quo, __rem;
	__asm__ (
#ifdef _WIN64
		"xor rdx, rdx \n"
		"div %3 \n"
		: "=a"(__quo), "=d"(__rem)
		: "a"(__a), "rm"((_MCFCRT_STD int64_t)__b)
#else
		"div %3 \n"
		: "=a"(__quo), "=d"(__rem)
		: "A"(__a), "rm"(__b)
#endif
	);
	if(__rem_ret){
		*__rem_ret = __rem;
	}
	return __quo;
}

_MCFCRT_CONSTEXPR __MCFCRT_DIV64_INLINE_OR_EXTERN _MCFCRT_STD uint32_t _MCFCRT_udiv64(_MCFCRT_STD uint64_t __a, _MCFCRT_STD uint32_t __b) _MCFCRT_NOEXCEPT {
	return __builtin_constant_p(__a / __b) ? (_MCFCRT_STD uint32_t)(__a / (_MCFCRT_STD uint64_t)__b)
	                                       : __MCFCRT_udiv64_nonconstexpr(_MCFCRT_NULLPTR, __a, __b);
}
__MCFCRT_DIV64_INLINE_OR_EXTERN _MCFCRT_STD uint32_t _MCFCRT_udivrem64(_MCFCRT_STD uint32_t *_MCFCRT_RESTRICT __rem_ret, _MCFCRT_STD uint64_t __a, _MCFCRT_STD uint32_t __b) _MCFCRT_NOEXCEPT {
	return __builtin_constant_p(__a / __b) ? ((__rem_ret ? (*__rem_ret = (_MCFCRT_STD uint32_t)(__a % (_MCFCRT_STD uint64_t)__b)) : 0), (_MCFCRT_STD uint32_t)(__a / (_MCFCRT_STD uint64_t)__b))
	                                       : __MCFCRT_udiv64_nonconstexpr(__rem_ret, __a, __b);
}

_MCFCRT_EXTERN_C_END

#endif
