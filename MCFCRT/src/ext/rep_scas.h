// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_EXT_REP_SCAS_H_
#define __MCFCRT_EXT_REP_SCAS_H_

#include "../env/_crtdef.h"

#ifndef __MCFCRT_REP_SCAS_INLINE_OR_EXTERN
#	define __MCFCRT_REP_SCAS_INLINE_OR_EXTERN     __attribute__((__gnu_inline__)) extern inline
#endif

_MCFCRT_EXTERN_C_BEGIN

__MCFCRT_REP_SCAS_INLINE_OR_EXTERN void *_MCFCRT_rep_scasb(const void *__s, _MCFCRT_STD uint8_t __c, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	const void *__p = __s;
	_MCFCRT_STD size_t __unused;
	__asm__ (
		"xor %2, %2 \n"
		"repnz scasb \n"
		"setz %b2 \n"
#ifdef _WIN64
		"neg %q2 \n"
		"add %q0, %q2 \n"
		"and %q0, %q2 \n"
#else
		"neg %k2 \n"
		"add %k0, %k2 \n"
		"and %k0, %k2 \n"
#endif
		: "+D"(__p), "=c"(__unused), "=&q"(__unused)
		: "m"(*(const char *)__s), "a"(__c), "c"(__n)
	);
	return (void *)__p;
}
__MCFCRT_REP_SCAS_INLINE_OR_EXTERN void *_MCFCRT_rep_scasw(const void *__s, _MCFCRT_STD uint16_t __c, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	const void *__p = __s;
	_MCFCRT_STD size_t __unused;
	__asm__ (
		"xor %2, %2 \n"
		"repnz scasw \n"
		"setz %b2 \n"
#ifdef _WIN64
		"neg %q2 \n"
		"add %q0, %q2 \n"
		"and %q0, %q2 \n"
#else
		"neg %k2 \n"
		"add %k0, %k2 \n"
		"and %k0, %k2 \n"
#endif
		: "+D"(__p), "=c"(__unused), "=&q"(__unused)
		: "m"(*(const char *)__s), "a"(__c), "c"(__n)
	);
	return (void *)__p;
}
__MCFCRT_REP_SCAS_INLINE_OR_EXTERN void *_MCFCRT_rep_scasd(const void *__s, _MCFCRT_STD uint32_t __c, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	const void *__p = __s;
	_MCFCRT_STD size_t __unused;
	__asm__ (
		"xor %2, %2 \n"
		"repnz scasd \n"
		"setz %b2 \n"
#ifdef _WIN64
		"neg %q2 \n"
		"add %q0, %q2 \n"
		"and %q0, %q2 \n"
#else
		"neg %k2 \n"
		"add %k0, %k2 \n"
		"and %k0, %k2 \n"
#endif
		: "+D"(__p), "=c"(__unused), "=&q"(__unused)
		: "m"(*(const char *)__s), "a"(__c), "c"(__n)
	);
	return (void *)__p;
}
#ifdef _WIN64
__MCFCRT_REP_SCAS_INLINE_OR_EXTERN void *_MCFCRT_rep_scasq(const void *__s, _MCFCRT_STD uint64_t __c, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	const void *__p = __s;
	_MCFCRT_STD size_t __unused;
	__asm__ (
		"xor %2, %2 \n"
		"repnz scasq \n"
		"setz %b2 \n"
		"neg %q2 \n"
		"add %q0, %q2 \n"
		"and %q0, %q2 \n"
		: "+D"(__p), "=c"(__unused), "=&q"(__unused)
		: "m"(*(const char *)__s), "a"(__c), "c"(__n)
	);
	return (void *)__p;
}
#endif

_MCFCRT_EXTERN_C_END

#endif
