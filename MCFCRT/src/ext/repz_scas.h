// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_EXT_REPZ_SCAS_H_
#define __MCFCRT_EXT_REPZ_SCAS_H_

#include "../env/_crtdef.h"

#ifndef __MCFCRT_REPZ_SCAS_INLINE_OR_EXTERN
#	define __MCFCRT_REPZ_SCAS_INLINE_OR_EXTERN     __attribute__((__gnu_inline__)) extern inline
#endif

_MCFCRT_EXTERN_C_BEGIN

__MCFCRT_REPZ_SCAS_INLINE_OR_EXTERN void *_MCFCRT_repz_scasb(const void *__s, int __c, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	const _MCFCRT_STD uint8_t *__p = (const _MCFCRT_STD uint8_t *)__s;
	_MCFCRT_STD size_t __unused;
	__asm__ (
		"xor %2, %2 \n"
		"repz scasb \n"
		"setnz %b2 \n"
#ifdef _WIN64
		"neg %q2 \n"
		"add rdi, %q2 \n"
		"and rdi, %q2 \n"
#else
		"neg %k2 \n"
		"add edi, %k2 \n"
		"and edi, %k2 \n"
#endif
		: "+D"(__p), "=c"(__unused), "=&q"(__unused)
		: "m"(*__p), "a"(__c), "c"(__n)
	);
	return (_MCFCRT_STD uint8_t *)__p;
}
__MCFCRT_REPZ_SCAS_INLINE_OR_EXTERN _MCFCRT_STD uint16_t *_MCFCRT_repz_scasw(const _MCFCRT_STD uint16_t *__s, _MCFCRT_STD uint16_t __c, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	const _MCFCRT_STD uint16_t *__p = __s;
	_MCFCRT_STD size_t __unused;
	__asm__ (
		"xor %2, %2 \n"
		"repz scasw \n"
		"setnz %b2 \n"
#ifdef _WIN64
		"neg %q2 \n"
		"add rdi, %q2 \n"
		"and rdi, %q2 \n"
#else
		"neg %k2 \n"
		"add edi, %k2 \n"
		"and edi, %k2 \n"
#endif
		: "+D"(__p), "=c"(__unused), "=&q"(__unused)
		: "m"(*__p), "a"(__c), "c"(__n)
	);
	return (_MCFCRT_STD uint16_t *)__p;
}
__MCFCRT_REPZ_SCAS_INLINE_OR_EXTERN _MCFCRT_STD uint32_t *_MCFCRT_repz_scasd(const _MCFCRT_STD uint32_t *__s, _MCFCRT_STD uint32_t __c, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	const _MCFCRT_STD uint32_t *__p = __s;
	_MCFCRT_STD size_t __unused;
	__asm__ (
		"xor %2, %2 \n"
		"repz scasd \n"
		"setnz %b2 \n"
#ifdef _WIN64
		"neg %q2 \n"
		"add rdi, %q2 \n"
		"and rdi, %q2 \n"
#else
		"neg %k2 \n"
		"add edi, %k2 \n"
		"and edi, %k2 \n"
#endif
		: "+D"(__p), "=c"(__unused), "=&q"(__unused)
		: "m"(*__p), "a"(__c), "c"(__n)
	);
	return (_MCFCRT_STD uint32_t *)__p;
}
#ifdef _WIN64
__MCFCRT_REPZ_SCAS_INLINE_OR_EXTERN _MCFCRT_STD uint64_t *_MCFCRT_repz_scasq(const _MCFCRT_STD uint64_t *__s, _MCFCRT_STD uint64_t __c, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	const _MCFCRT_STD uint64_t *__p = __s;
	_MCFCRT_STD size_t __unused;
	__asm__ (
		"xor %2, %2 \n"
		"repz scasq \n"
		"setnz %b2 \n"
		"neg %q2 \n"
		"add rdi, %q2 \n"
		"and rdi, %q2 \n"
		: "+D"(__p), "=c"(__unused), "=&q"(__unused)
		: "m"(*__p), "a"(__c), "c"(__n)
	);
	return (_MCFCRT_STD uint64_t *)__p;
}
#endif

_MCFCRT_EXTERN_C_END

#endif
