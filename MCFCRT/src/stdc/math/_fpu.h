// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_MATH_FPU_H_
#define __MCFCRT_MATH_FPU_H_

#include "../../env/_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

static inline long double __MCFCRT_fsqrt(long double __x) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__("fsqrt \n" : "=&t"(__ret) : "0"(__x));
	return __ret;
}

static inline long double __MCFCRT_fpatan(long double __y, long double __x) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__("fpatan \n" : "=&t"(__ret) : "0"(__x), "u"(__y));
	return __ret;
}

_MCFCRT_EXTERN_C_END

#endif
