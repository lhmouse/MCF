// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_STDC_MATH_SSE3_H_
#define __MCFCRT_STDC_MATH_SSE3_H_

#include "../../env/_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

static inline void __MCFCRT_fisttp(int *__p, long double __x) _MCFCRT_NOEXCEPT {
	__asm__ (
		"fisttp %0 \n"
		: "=m"(*__p)
		: "t"(__x)
		: "st"
	);
}
static inline void __MCFCRT_fisttpl(long *__p, long double __x) _MCFCRT_NOEXCEPT {
	__asm__ (
		"fisttp %0 \n"
		: "=m"(*__p)
		: "t"(__x)
		: "st"
	);
}
static inline void __MCFCRT_fisttpll(long long *__p, long double __x) _MCFCRT_NOEXCEPT {
	__asm__ (
		"fisttp %0 \n"
		: "=m"(*__p)
		: "t"(__x)
		: "st"
	);
}

_MCFCRT_EXTERN_C_END

#endif
