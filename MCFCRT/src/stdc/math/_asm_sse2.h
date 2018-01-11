// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_STDC_MATH_ASM_SSE2_H_
#define __MCFCRT_STDC_MATH_ASM_SSE2_H_

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
		"sqrtss %0, %1 \n"
		: "=x"(__ret)
		: "x"(__x)
	);
	return __ret;
}
static inline double __MCFCRT_xmmsqrtsd(double __x) _MCFCRT_NOEXCEPT {
	double __ret;
	__asm__ (
		"sqrtsd %0, %1 \n"
		: "=x"(__ret)
		: "x"(__x)
	);
	return __ret;
}

static inline bool __MCFCRT_xmmsignbitss(float __x) _MCFCRT_NOEXCEPT {
	int __mask;
	__asm__ (
		"movmskps %0, %1 \n"
		: "=r"(__mask)
		: "x"(__x)
	);
	return __mask & 1;
}
static inline bool __MCFCRT_xmmsignbitsd(double __x) _MCFCRT_NOEXCEPT {
	int __mask;
	__asm__ (
		"movmskpd %0, %1 \n"
		: "=r"(__mask)
		: "x"(__x)
	);
	return __mask & 1;
}

static inline float __MCFCRT_xmmabsss(float __x) _MCFCRT_NOEXCEPT {
	float __ret;
	__asm__ (
		"andnps %0, %2 \n"
		: "=x"(__ret)
		: "0"(-(float)0), "x"(__x)
	);
	return __ret;
}
static inline double __MCFCRT_xmmabssd(double __x) _MCFCRT_NOEXCEPT {
	double __ret;
	__asm__ (
		"andnpd %0, %2 \n"
		: "=x"(__ret)
		: "0"(-(double)0), "x"(__x)
	);
	return __ret;
}

static inline float __MCFCRT_xmmcopysignss(float __x, float __y) _MCFCRT_NOEXCEPT {
	float __ret, __dumb;
	__asm__ (
		"xorps %1, %0 \n"
		"andps %1, %4 \n"
		"xorps %0, %1 \n"
		: "=&x"(__ret), "=&x"(__dumb)
		: "0"(__x), "1"(__y), "x"(-(float)0)
	);
	return __ret;
}
static inline double __MCFCRT_xmmcopysignsd(double __x, double __y) _MCFCRT_NOEXCEPT {
	double __ret, __dumb;
	__asm__ (
		"xorpd %1, %0 \n"
		"andpd %1, %4 \n"
		"xorpd %0, %1 \n"
		: "=&x"(__ret), "=&x"(__dumb)
		: "0"(__x), "1"(__y), "x"(-(double)0)
	);
	return __ret;
}

_MCFCRT_EXTERN_C_END

#endif
