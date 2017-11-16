// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_STDC_MATH_SSE2_H_
#define __MCFCRT_STDC_MATH_SSE2_H_

#include "../../env/_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

static inline float __MCFCRT_xmmminss(float __x, float __y) _MCFCRT_NOEXCEPT {
	float __ret;
	__asm__ (
		"minss %0, %2 \n"
		: "=x"(__ret)
		: "0"(__x), "x"(__y)
	);
	return __ret;
}
static inline double __MCFCRT_xmmminsd(double __x, double __y) _MCFCRT_NOEXCEPT {
	double __ret;
	__asm__ (
		"minsd %0, %2 \n"
		: "=x"(__ret)
		: "0"(__x), "x"(__y)
	);
	return __ret;
}

static inline float __MCFCRT_xmmmaxss(float __x, float __y) _MCFCRT_NOEXCEPT {
	float __ret;
	__asm__ (
		"maxss %0, %2 \n"
		: "=x"(__ret)
		: "0"(__x), "x"(__y)
	);
	return __ret;
}
static inline double __MCFCRT_xmmmaxsd(double __x, double __y) _MCFCRT_NOEXCEPT {
	double __ret;
	__asm__ (
		"maxsd %0, %2 \n"
		: "=x"(__ret)
		: "0"(__x), "x"(__y)
	);
	return __ret;
}

static inline float __MCFCRT_xmmsqrtss(float __x) _MCFCRT_NOEXCEPT {
	float __ret;
	__asm__ (
		"sqrtss %0, %0 \n"
		: "=x"(__ret)
		: "0"(__x)
	);
	return __ret;
}
static inline double __MCFCRT_xmmsqrtsd(double __x) _MCFCRT_NOEXCEPT {
	double __ret;
	__asm__ (
		"sqrtsd %0, %0 \n"
		: "=x"(__ret)
		: "0"(__x)
	);
	return __ret;
}

static inline bool __MCFCRT_xmmsignbitss(float __x) _MCFCRT_NOEXCEPT {
	bool __ret;
	__asm__ (
		"movmskps eax, %1 \n"
		"and eax, 1 \n"
		: "=a"(__ret)
		: "x"(__x)
	);
	return __ret;
}
static inline bool __MCFCRT_xmmsignbitsd(double __x) _MCFCRT_NOEXCEPT {
	bool __ret;
	__asm__ (
		"movmskpd eax, %1 \n"
		"and eax, 1 \n"
		: "=a"(__ret)
		: "x"(__x)
	);
	return __ret;
}

_MCFCRT_EXTERN_C_END

#endif
