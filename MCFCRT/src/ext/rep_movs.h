// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_EXT_REP_MOVS_H_
#define __MCFCRT_EXT_REP_MOVS_H_

#include "../env/_crtdef.h"
#include "../env/xassert.h"

#ifndef __MCFCRT_REP_MOVS_INLINE_OR_EXTERN
#	define __MCFCRT_REP_MOVS_INLINE_OR_EXTERN     __attribute__((__gnu_inline__)) extern inline
#endif

_MCFCRT_EXTERN_C_BEGIN

__MCFCRT_REP_MOVS_INLINE_OR_EXTERN void *_MCFCRT_rep_movsb(void *__s1, const void *__s2, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	_MCFCRT_STD uint8_t *__p1 = (_MCFCRT_STD uint8_t *)__s1;
	const _MCFCRT_STD uint8_t *__p2 = (const _MCFCRT_STD uint8_t *)__s2;
	_MCFCRT_STD size_t __unused;
	__asm__ (
		"rep movsb \n"
		: "=m"(*__p1), "+D"(__p1), "+S"(__p2), "=c"(__unused)
		: "m"(*__p2), "c"(__n)
	);
	_MCFCRT_ASSERT(__p1 == (_MCFCRT_STD uint8_t *)__s1 + __n);
	_MCFCRT_ASSERT(__p2 == (_MCFCRT_STD uint8_t *)__s2 + __n);
	return __p1;
}
__MCFCRT_REP_MOVS_INLINE_OR_EXTERN _MCFCRT_STD uint16_t *_MCFCRT_rep_movsw(_MCFCRT_STD uint16_t *__s1, const _MCFCRT_STD uint16_t *__s2, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	_MCFCRT_STD uint16_t *__p1 = __s1;
	const _MCFCRT_STD uint16_t *__p2 = __s2;
	_MCFCRT_STD size_t __unused;
	__asm__ (
		"rep movsw \n"
		: "=m"(*__p1), "+D"(__p1), "+S"(__p2), "=c"(__unused)
		: "m"(*__p2), "c"(__n)
	);
	_MCFCRT_ASSERT(__p1 == __s1 + __n);
	_MCFCRT_ASSERT(__p2 == __s2 + __n);
	return __p1;
}
__MCFCRT_REP_MOVS_INLINE_OR_EXTERN _MCFCRT_STD uint32_t *_MCFCRT_rep_movsd(_MCFCRT_STD uint32_t *__s1, const _MCFCRT_STD uint32_t *__s2, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	_MCFCRT_STD uint32_t *__p1 = __s1;
	const _MCFCRT_STD uint32_t *__p2 = __s2;
	_MCFCRT_STD size_t __unused;
	__asm__ (
		"rep movsd \n"
		: "=m"(*__p1), "+D"(__p1), "+S"(__p2), "=c"(__unused)
		: "m"(*__p2), "c"(__n)
	);
	_MCFCRT_ASSERT(__p1 == __s1 + __n);
	_MCFCRT_ASSERT(__p2 == __s2 + __n);
	return __p1;
}
#ifdef _WIN64
__MCFCRT_REP_MOVS_INLINE_OR_EXTERN _MCFCRT_STD uint64_t *_MCFCRT_rep_movsq(_MCFCRT_STD uint64_t *__s1, const _MCFCRT_STD uint64_t *__s2, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	_MCFCRT_STD uint64_t *__p1 = __s1;
	const _MCFCRT_STD uint64_t *__p2 = __s2;
	_MCFCRT_STD size_t __unused;
	__asm__ (
		"rep movsq \n"
		: "=m"(*__p1), "+D"(__p1), "+S"(__p2), "=c"(__unused)
		: "m"(*__p2), "c"(__n)
	);
	_MCFCRT_ASSERT(__p1 == __s1 + __n);
	_MCFCRT_ASSERT(__p2 == __s2 + __n);
	return __p1;
}
#endif

_MCFCRT_EXTERN_C_END

#endif
