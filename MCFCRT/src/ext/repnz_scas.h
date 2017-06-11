// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_EXT_REPNZ_SCAS_H_
#define __MCFCRT_EXT_REPNZ_SCAS_H_

#include "../env/_crtdef.h"

#ifndef __MCFCRT_REPNZ_SCAS_INLINE_OR_EXTERN
#  define __MCFCRT_REPNZ_SCAS_INLINE_OR_EXTERN     __attribute__((__gnu_inline__)) extern inline
#endif

_MCFCRT_EXTERN_C_BEGIN

__MCFCRT_REPNZ_SCAS_INLINE_OR_EXTERN int _MCFCRT_repnz_scasb(void **_MCFCRT_RESTRICT __es, const void *__s, int __c, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	int __z, __b;
	const _MCFCRT_STD uint8_t *__p = (const _MCFCRT_STD uint8_t *)__s;
	__asm__ (
		"repnz scasb \n"
		: "+D"(__p), "=@ccz"(__z), "=@ccb"(__b)
		: "m"(*__p), "a"(__c), "c"(__n)
	);
	if(__es){
		*__es = (_MCFCRT_STD uint8_t *)(__p - __z);
	}
	return (__z - 1) & ((__b - 1) | 1);
}
__MCFCRT_REPNZ_SCAS_INLINE_OR_EXTERN int _MCFCRT_repnz_scasw(_MCFCRT_STD uint16_t **_MCFCRT_RESTRICT __es, const _MCFCRT_STD uint16_t *__s, int __c, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	int __z, __b;
	const _MCFCRT_STD uint16_t *__p = __s;
	__asm__ (
		"repnz scasw \n"
		: "+D"(__p), "=@ccz"(__z), "=@ccb"(__b)
		: "m"(*__p), "a"(__c), "c"(__n)
	);
	if(__es){
		*__es = (_MCFCRT_STD uint16_t *)(__p - __z);
	}
	return (__z - 1) & ((__b - 1) | 1);
}
__MCFCRT_REPNZ_SCAS_INLINE_OR_EXTERN int _MCFCRT_repnz_scasd(_MCFCRT_STD uint32_t **_MCFCRT_RESTRICT __es, const _MCFCRT_STD uint32_t *__s, int __c, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	int __z, __b;
	const _MCFCRT_STD uint32_t *__p = __s;
	__asm__ (
		"repnz scasd \n"
		: "+D"(__p), "=@ccz"(__z), "=@ccb"(__b)
		: "m"(*__p), "a"(__c), "c"(__n)
	);
	if(__es){
		*__es = (_MCFCRT_STD uint32_t *)(__p - __z);
	}
	return (__z - 1) & ((__b - 1) | 1);
}
#ifdef _WIN64
__MCFCRT_REPNZ_SCAS_INLINE_OR_EXTERN int _MCFCRT_repnz_scasq(_MCFCRT_STD uint64_t **_MCFCRT_RESTRICT __es, const _MCFCRT_STD uint64_t *__s, int __c, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	int __z, __b;
	const _MCFCRT_STD uint64_t *__p = __s;
	__asm__ (
		"repnz scasq \n"
		: "+D"(__p), "=@ccz"(__z), "=@ccb"(__b)
		: "m"(*__p), "a"(__c), "c"(__n)
	);
	if(__es){
		*__es = (_MCFCRT_STD uint64_t *)(__p - __z);
	}
	return (__z - 1) & ((__b - 1) | 1);
}
#endif

_MCFCRT_EXTERN_C_END

#endif
