// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_INLINE_MEM_H_
#define __MCFCRT_ENV_INLINE_MEM_H_

#include "_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

__attribute__((__always_inline__))
static inline void *_MCFCRT_inline_mempcpy_fwd(void *__s1, const void *__s2, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	void *__p1 = __s1;
	const void *__p2 = __s2;
	_MCFCRT_STD size_t __unused;
	__asm__ (
#ifdef _WIN64
		"mov rdx, rcx \n"
		"shr rcx, 3 \n"
		"rep movsq \n"
		"mov ecx, edx \n"
		"and ecx, 7 \n"
#else
		"mov edx, ecx \n"
		"shr ecx, 2 \n"
		"rep movsd \n"
		"mov ecx, edx \n"
		"and ecx, 3 \n"
#endif
		"rep movsb \n"
		: "=m"(*(char *)__s1), "+D"(__p1), "+S"(__p2), "=c"(__unused)
		: "m"(*(const char *)__s2), "c"(__n)
		: "dx"
	);
	return __p1;
}

__attribute__((__always_inline__))
static inline void *_MCFCRT_inline_mempset_fwd(void *__s, int __c, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	void *__p = __s;
	_MCFCRT_STD uintptr_t __word = __c & 0xFF;
	__word += __word <<  8;
	__word += __word << 16;
#ifdef _WIN64
	__word += __word << 32;
#endif
	_MCFCRT_STD size_t __unused;
	__asm__ (
#ifdef _WIN64
		"mov rdx, rcx \n"
		"shr rcx, 3 \n"
		"rep stosq \n"
		"mov ecx, edx \n"
		"and ecx, 7 \n"
#else
		"mov edx, ecx \n"
		"shr ecx, 2 \n"
		"rep stosd \n"
		"mov ecx, edx \n"
		"and ecx, 3 \n"
#endif
		"rep stosb \n"
		: "=m"(*(char *)__s), "+D"(__p), "=c"(__unused)
		: "a"(__word), "c"(__n)
		: "dx"
	);
	return __p;
}

_MCFCRT_EXTERN_C_END

#endif
