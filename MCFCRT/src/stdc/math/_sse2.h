// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_STDC_MATH_SSE2_H_
#define __MCFCRT_STDC_MATH_SSE2_H_

#include "../../env/_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

static inline float __MCFCRT_minss(float __x, float __y) _MCFCRT_NOEXCEPT {
	float __ret;
	__asm__ (
		"minss %0, %2 \n"
		: "=x"(__ret)
		: "0"(__x), "x"(__y)
	);
	return __ret;
}
static inline double __MCFCRT_minsd(double __x, double __y) _MCFCRT_NOEXCEPT {
	double __ret;
	__asm__ (
		"minsd %0, %2 \n"
		: "=x"(__ret)
		: "0"(__x), "x"(__y)
	);
	return __ret;
}

static inline float __MCFCRT_maxss(float __x, float __y) _MCFCRT_NOEXCEPT {
	float __ret;
	__asm__ (
		"maxss %0, %2 \n"
		: "=x"(__ret)
		: "0"(__x), "x"(__y)
	);
	return __ret;
}
static inline double __MCFCRT_maxsd(double __x, double __y) _MCFCRT_NOEXCEPT {
	double __ret;
	__asm__ (
		"maxsd %0, %2 \n"
		: "=x"(__ret)
		: "0"(__x), "x"(__y)
	);
	return __ret;
}

static inline float __MCFCRT_sqrtss(float __x) _MCFCRT_NOEXCEPT {
	float __ret;
	__asm__ (
		"sqrtss %0, %0 \n"
		: "=x"(__ret)
		: "0"(__x)
	);
	return __ret;
}
static inline double __MCFCRT_sqrtsd(double __x) _MCFCRT_NOEXCEPT {
	double __ret;
	__asm__ (
		"sqrtsd %0, %0 \n"
		: "=x"(__ret)
		: "0"(__x)
	);
	return __ret;
}

_MCFCRT_EXTERN_C_END

#endif
