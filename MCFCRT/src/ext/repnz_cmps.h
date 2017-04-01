// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_EXT_REPNZ_CMPS_H_
#define __MCFCRT_EXT_REPNZ_CMPS_H_

#include "../env/_crtdef.h"

#ifndef __MCFCRT_REPNZ_CMPS_INLINE_OR_EXTERN
#	define __MCFCRT_REPNZ_CMPS_INLINE_OR_EXTERN     __attribute__((__gnu_inline__)) extern inline
#endif

_MCFCRT_EXTERN_C_BEGIN

__MCFCRT_REPNZ_CMPS_INLINE_OR_EXTERN int _MCFCRT_repnz_cmpsb(void **_MCFCRT_RESTRICT __eptr, const void *__s1, const void *__s2, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	int __r;
	const _MCFCRT_STD uint8_t *__p1 = (const _MCFCRT_STD uint8_t *)__s1;
	const _MCFCRT_STD uint8_t *__p2 = (const _MCFCRT_STD uint8_t *)__s2;
	_MCFCRT_STD size_t __unused;
	__asm__ (
		"xor %3, %3 \n"
		"repnz cmpsb \n"
		"mov ecx, %k3 \n"
		"setnz %b3 \n"
		"setb cl \n"
		"dec ecx \n"
		"or ecx, 1 \n"
		"neg %k3 \n"
		"and %k3, ecx \n"
		: "+D"(__p1), "+S"(__p2), "=c"(__unused), "=&q"(__r)
		: "m"(*__p1), "m"(*__p2), "c"(__n)
	);
	if(__eptr){
		*__eptr = (_MCFCRT_STD uint8_t *)(__p1 - (__r == 0));
	}
	return __r;
}
__MCFCRT_REPNZ_CMPS_INLINE_OR_EXTERN int _MCFCRT_repnz_cmpsw(_MCFCRT_STD uint16_t **_MCFCRT_RESTRICT __eptr, const _MCFCRT_STD uint16_t *__s1, const _MCFCRT_STD uint16_t *__s2, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	int __r;
	const _MCFCRT_STD uint16_t *__p1 = __s1;
	const _MCFCRT_STD uint16_t *__p2 = __s2;
	_MCFCRT_STD size_t __unused;
	__asm__ (
		"xor %3, %3 \n"
		"repnz cmpsw \n"
		"mov ecx, %k3 \n"
		"setnz %b3 \n"
		"setb cl \n"
		"dec ecx \n"
		"or ecx, 1 \n"
		"neg %k3 \n"
		"and %k3, ecx \n"
		: "+D"(__p1), "+S"(__p2), "=c"(__unused), "=&q"(__r)
		: "m"(*__p1), "m"(*__p2), "c"(__n)
	);
	if(__eptr){
		*__eptr = (_MCFCRT_STD uint16_t *)(__p1 - (__r == 0));
	}
	return __r;
}
__MCFCRT_REPNZ_CMPS_INLINE_OR_EXTERN int _MCFCRT_repnz_cmpsd(_MCFCRT_STD uint32_t **_MCFCRT_RESTRICT __eptr, const _MCFCRT_STD uint32_t *__s1, const _MCFCRT_STD uint32_t *__s2, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	int __r;
	const _MCFCRT_STD uint32_t *__p1 = __s1;
	const _MCFCRT_STD uint32_t *__p2 = __s2;
	_MCFCRT_STD size_t __unused;
	__asm__ (
		"xor %3, %3 \n"
		"repnz cmpsd \n"
		"mov ecx, %k3 \n"
		"setnz %b3 \n"
		"setb cl \n"
		"dec ecx \n"
		"or ecx, 1 \n"
		"neg %k3 \n"
		"and %k3, ecx \n"
		: "+D"(__p1), "+S"(__p2), "=c"(__unused), "=&q"(__r)
		: "m"(*__p1), "m"(*__p2), "c"(__n)
	);
	if(__eptr){
		*__eptr = (_MCFCRT_STD uint32_t *)(__p1 - (__r == 0));
	}
	return __r;
}
#ifdef _WIN64
__MCFCRT_REPNZ_CMPS_INLINE_OR_EXTERN int _MCFCRT_repnz_cmpsq(_MCFCRT_STD uint64_t **_MCFCRT_RESTRICT __eptr, const _MCFCRT_STD uint64_t *__s1, const _MCFCRT_STD uint64_t *__s2, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	int __r;
	const _MCFCRT_STD uint64_t *__p1 = __s1;
	const _MCFCRT_STD uint64_t *__p2 = __s2;
	_MCFCRT_STD size_t __unused;
	__asm__ (
		"xor %3, %3 \n"
		"repnz cmpsq \n"
		"mov ecx, %k3 \n"
		"setnz %b3 \n"
		"setb cl \n"
		"dec ecx \n"
		"or ecx, 1 \n"
		"neg %k3 \n"
		"and %k3, ecx \n"
		: "+D"(__p1), "+S"(__p2), "=c"(__unused), "=&q"(__r)
		: "m"(*__p1), "m"(*__p2), "c"(__n)
	);
	if(__eptr){
		*__eptr = (_MCFCRT_STD uint64_t *)(__p1 - (__r == 0));
	}
	return __r;
}
#endif

_MCFCRT_EXTERN_C_END

#endif
