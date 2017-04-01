// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_EXT_REPZ_CMPS_H_
#define __MCFCRT_EXT_REPZ_CMPS_H_

#include "../env/_crtdef.h"

#ifndef __MCFCRT_REPZ_CMPS_INLINE_OR_EXTERN
#	define __MCFCRT_REPZ_CMPS_INLINE_OR_EXTERN     __attribute__((__gnu_inline__)) extern inline
#endif

_MCFCRT_EXTERN_C_BEGIN

__MCFCRT_REPZ_CMPS_INLINE_OR_EXTERN int _MCFCRT_repz_cmpsb(void **_MCFCRT_RESTRICT __es1, const void *__s1, const void *__s2, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	int __z, __b;
	const _MCFCRT_STD uint8_t *__p1 = (const _MCFCRT_STD uint8_t *)__s1;
	const _MCFCRT_STD uint8_t *__p2 = (const _MCFCRT_STD uint8_t *)__s2;
	__asm__ (
		"xor %3, %3 \n"
		"repz cmpsb \n"
		"mov ecx, %k3 \n"
		"setz cl \n"
		"setb %b3 \n"
		: "+D"(__p1), "+S"(__p2), "=c"(__z), "=&r"(__b)
		: "m"(*__p1), "m"(*__p2), "c"(__n)
	);
	if(__es1){
		*__es1 = (_MCFCRT_STD uint8_t *)(__p1 + __z - 1);
	}
	return (__z - 1) & ((__b - 1) | 1);
}
__MCFCRT_REPZ_CMPS_INLINE_OR_EXTERN int _MCFCRT_repz_cmpsw(_MCFCRT_STD uint16_t **_MCFCRT_RESTRICT __es1, const _MCFCRT_STD uint16_t *__s1, const _MCFCRT_STD uint16_t *__s2, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	int __z, __b;
	const _MCFCRT_STD uint16_t *__p1 = __s1;
	const _MCFCRT_STD uint16_t *__p2 = __s2;
	__asm__ (
		"xor %3, %3 \n"
		"repz cmpsw \n"
		"mov ecx, %k3 \n"
		"setz cl \n"
		"setb %b3 \n"
		: "+D"(__p1), "+S"(__p2), "=c"(__z), "=&r"(__b)
		: "m"(*__p1), "m"(*__p2), "c"(__n)
	);
	if(__es1){
		*__es1 = (_MCFCRT_STD uint16_t *)(__p1 + __z - 1);
	}
	return (__z - 1) & ((__b - 1) | 1);
}
__MCFCRT_REPZ_CMPS_INLINE_OR_EXTERN int _MCFCRT_repz_cmpsd(_MCFCRT_STD uint32_t **_MCFCRT_RESTRICT __es1, const _MCFCRT_STD uint32_t *__s1, const _MCFCRT_STD uint32_t *__s2, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	int __z, __b;
	const _MCFCRT_STD uint32_t *__p1 = __s1;
	const _MCFCRT_STD uint32_t *__p2 = __s2;
	__asm__ (
		"xor %3, %3 \n"
		"repz cmpsd \n"
		"mov ecx, %k3 \n"
		"setz cl \n"
		"setb %b3 \n"
		: "+D"(__p1), "+S"(__p2), "=c"(__z), "=&r"(__b)
		: "m"(*__p1), "m"(*__p2), "c"(__n)
	);
	if(__es1){
		*__es1 = (_MCFCRT_STD uint32_t *)(__p1 + __z - 1);
	}
	return (__z - 1) & ((__b - 1) | 1);
}
#ifdef _WIN64
__MCFCRT_REPZ_CMPS_INLINE_OR_EXTERN int _MCFCRT_repz_cmpsq(_MCFCRT_STD uint64_t **_MCFCRT_RESTRICT __es1, const _MCFCRT_STD uint64_t *__s1, const _MCFCRT_STD uint64_t *__s2, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	int __z, __b;
	const _MCFCRT_STD uint64_t *__p1 = __s1;
	const _MCFCRT_STD uint64_t *__p2 = __s2;
	__asm__ (
		"xor %3, %3 \n"
		"repz cmpsq \n"
		"mov ecx, %k3 \n"
		"setz cl \n"
		"setb %b3 \n"
		: "+D"(__p1), "+S"(__p2), "=c"(__z), "=&r"(__b)
		: "m"(*__p1), "m"(*__p2), "c"(__n)
	);
	if(__es1){
		*__es1 = (_MCFCRT_STD uint64_t *)(__p1 + __z - 1);
	}
	return (__z - 1) & ((__b - 1) | 1);
}
#endif

_MCFCRT_EXTERN_C_END

#endif
