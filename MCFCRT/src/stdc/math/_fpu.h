// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_MATH_FPU_H_
#define __MCFCRT_MATH_FPU_H_

#include "../../env/_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

typedef enum __MCFCRT_tagFpuSign {
	__MCFCRT_kFpuPositive  = 0x00,
	__MCFCRT_kFpuNegative  = 0x01,
	__MCFCRT_kFpuZero      = 0x40,
	__MCFCRT_kFpuUnordered = 0x41,
} __MCFCRT_FpuSign;

static inline __MCFCRT_FpuSign __MCFCRT_ftest(long double __x) _MCFCRT_NOEXCEPT {
	__MCFCRT_FpuSign __ret;
	__asm__(
		"ftst \n"
		"fstsw ax \n"
		"shr eax, 8 \n"
		"and eax, 0x41 \n"
		: "=a"(__ret)
		: "t"(__x)
	);
	return __ret;
}

static inline bool __MCFCRT_fgetsign(long double __x) _MCFCRT_NOEXCEPT {
	bool __ret;
	__asm__(
		"fxam \n"
		"fstsw ax \n"
		"shr eax, 9 \n"
		"and eax, 0x01 \n"
		: "=a"(__ret)
		: "t"(__x)
	);
	return __ret;
}

static inline long double __MCFCRT_fabs(long double __x) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__(
		"fabs \n"
		: "=&t"(__ret)
		: "0"(__x)
	);
	return __ret;
}
static inline long double __MCFCRT_fneg(long double __x) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__(
		"fchs \n"
		: "=&t"(__ret)
		: "0"(__x)
	);
	return __ret;
}

static inline void __MCFCRT_fistp(int *__p, long double __x) _MCFCRT_NOEXCEPT {
	__asm__(
		"fistp dword ptr[%0] \n"
		: "=m"(*__p)
		: "t"(__x)
		: "st"
	);
}
static inline void __MCFCRT_fistpl(long *__p, long double __x) _MCFCRT_NOEXCEPT {
	__asm__(
		"fistp dword ptr[%0] \n"
		: "=m"(*__p)
		: "t"(__x)
		: "st"
	);
}
static inline void __MCFCRT_fistpll(long long *__p, long double __x) _MCFCRT_NOEXCEPT {
	__asm__(
		"fistp qword ptr[%0] \n"
		: "=m"(*__p)
		: "t"(__x)
		: "st"
	);
}

static inline long double __MCFCRT_frndintany(long double __x) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__(
		"frndint \n"
		: "=&t"(__ret)
		: "0"(__x)
	);
	return __ret;
}
static inline long double __MCFCRT_ftrunc(long double __x) _MCFCRT_NOEXCEPT {
	long double __ret;
	_MCFCRT_STD uint16_t __fcw;
	__asm__(
		"fstcw word ptr[%1] \n"
		"movzx ecx, word ptr[%1] \n"
		"mov edx, ecx \n"
		"or edx, 0x0C00 \n"
		"mov word ptr[%1], dx \n"
		"fldcw word ptr[%1] \n"
		"frndint \n"
		"mov word ptr[%1], cx \n"
		"fldcw word ptr[%1] \n"
		: "=&t"(__ret), "=m"(__fcw)
		: "0"(__x)
		: "cx", "dx"
	);
	return __ret;
}
static inline long double __MCFCRT_fceil(long double __x) _MCFCRT_NOEXCEPT {
	long double __ret;
	_MCFCRT_STD uint16_t __fcw;
	__asm__(
		"fstcw word ptr[%1] \n"
		"movzx ecx, word ptr[%1] \n"
		"mov edx, ecx \n"
		"and edx, 0xF3FF \n"
		"or edx, 0x0800 \n"
		"mov word ptr[%1], dx \n"
		"fldcw word ptr[%1] \n"
		"frndint \n"
		"mov word ptr[%1], cx \n"
		"fldcw word ptr[%1] \n"
		: "=&t"(__ret), "=m"(__fcw)
		: "0"(__x)
		: "cx", "dx"
	);
	return __ret;
}
static inline long double __MCFCRT_ffloor(long double __x) _MCFCRT_NOEXCEPT {
	long double __ret;
	_MCFCRT_STD uint16_t __fcw;
	__asm__(
		"fstcw word ptr[%1] \n"
		"movzx ecx, word ptr[%1] \n"
		"mov edx, ecx \n"
		"and edx, 0xF3FF \n"
		"or edx, 0x0400 \n"
		"mov word ptr[%1], dx \n"
		"fldcw word ptr[%1] \n"
		"frndint \n"
		"mov word ptr[%1], cx \n"
		"fldcw word ptr[%1] \n"
		: "=&t"(__ret), "=m"(__fcw)
		: "0"(__x)
		: "cx", "dx"
	);
	return __ret;
}

static inline long double __MCFCRT_fsqrt(long double __x) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__(
		"fsqrt \n"
		: "=&t"(__ret)
		: "0"(__x)
	);
	return __ret;
}

static inline long double __MCFCRT_fmod(unsigned *__fsw, long double __x, long double __y) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__(
		"1: \n"
		"	fprem \n"
		"	fstsw ax \n"
		"	test ah, 0x04 \n"
		"	jnz 1b \n"
		: "=&t"(__ret), "=a"(*__fsw)
		: "0"(__x), "u"(__y)
	);
	return __ret;
}
static inline long double __MCFCRT_fremainder(unsigned *__fsw, long double __x, long double __y) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__(
		"1: \n"
		"	fprem1 \n"
		"	fstsw ax \n"
		"	test ah, 0x04 \n"
		"	jnz 1b \n"
		: "=&t"(__ret), "=a"(*__fsw)
		: "0"(__x), "u"(__y)
	);
	return __ret;
}

__attribute__((__cold__))
static inline long double __MCFCRT_trigonometric_reduce(long double __x) _MCFCRT_NOEXCEPT {
	unsigned __fsw;
	return __MCFCRT_fmod(&__fsw, __x, 0x1p61l * 3.1415926535897932384626433832795l);
}

static inline long double __MCFCRT_fsin(bool *__invalid, long double __x) _MCFCRT_NOEXCEPT {
	long double __ret;
	unsigned __fsw;
	__asm__(
		"fsin \n"
		"fstsw ax \n"
		: "=&t"(__ret), "=a"(__fsw)
		: "0"(__x)
	);
	*__invalid = (__fsw >> 10) & 1;
	return __ret;
}
static inline long double __MCFCRT_fcos(bool *__invalid, long double __x) _MCFCRT_NOEXCEPT {
	long double __ret;
	unsigned __fsw;
	__asm__(
		"fcos \n"
		"fstsw ax \n"
		: "=&t"(__ret), "=a"(__fsw)
		: "0"(__x)
	);
	*__invalid = (__fsw >> 10) & 1;
	return __ret;
}
static inline void __MCFCRT_fsincos(bool *__invalid, long double *__sinx, long double *__cosx, long double __x) _MCFCRT_NOEXCEPT {
	unsigned __fsw;
	__asm__(
		"fsincos \n"
		"fstsw ax \n"
		: "=&t"(*__cosx), "=&u"(*__sinx), "=a"(__fsw)
		: "0"(__x)
	);
	*__invalid = (__fsw >> 10) & 1;
}
static inline long double __MCFCRT_ftan(bool *__invalid, long double __x) _MCFCRT_NOEXCEPT {
	long double __ret;
	unsigned __fsw;
	__asm__(
		"fptan \n"
		"fstsw ax \n"
		"test ah, 0x04 \n"
		"jnz 1f \n"
		"	fstp st \n"
		"1: \n"
		: "=&t"(__ret), "=a"(__fsw)
		: "0"(__x)
	);
	*__invalid = (__fsw >> 10) & 1;
	return __ret;
}

static inline long double __MCFCRT_fpatan(long double __y, long double __x) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__(
		"fpatan \n"
		: "=&t"(__ret)
		: "0"(__x), "u"(__y)
		: "st(1)"
	);
	return __ret;
}

static inline long double __MCFCRT_fxtract(long double *__pn, long double __x) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__(
		"fxtract \n"
		: "=&t"(__ret), "=&u"(*__pn)
		: "0"(__x)
	);
	return __ret;
}

static inline long double __MCFCRT_fscale(long double __x, long double __n) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__(
		"fscale \n"
		: "=&t"(__ret)
		: "0"(__x), "u"(__n)
	);
	return __ret;
}
static inline long double __MCFCRT_fyl2x(long double __y, long double __x) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__(
		"fyl2x \n"
		: "=&t"(__ret)
		: "0"(__x), "u"(__y)
		: "st(1)"
	);
	return __ret;
}

static inline long double __MCFCRT_f2xm1(long double __x) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__(
		"f2xm1 \n"
		: "=&t"(__ret)
		: "0"(__x)
	);
	return __ret;
}
static inline long double __MCFCRT_fyl2xp1(long double __y, long double __x) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__(
		"fyl2xp1 \n"
		: "=&t"(__ret)
		: "0"(__x), "u"(__y)
		: "st(1)"
	);
	return __ret;
}

static inline long double __MCFCRT_fldl2e(void) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__(
		"fldl2e \n"
		: "=&t"(__ret)
	);
	return __ret;
}
static inline long double __MCFCRT_fldl2t(void) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__(
		"fldl2t \n"
		: "=&t"(__ret)
	);
	return __ret;
}
static inline long double __MCFCRT_fldlg2(void) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__(
		"fldlg2 \n"
		: "=&t"(__ret)
	);
	return __ret;
}
static inline long double __MCFCRT_fldln2(void) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__(
		"fldln2 \n"
		: "=&t"(__ret)
	);
	return __ret;
}
static inline long double __MCFCRT_fldpi(void) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__(
		"fldpi \n"
		: "=&t"(__ret)
	);
	return __ret;
}

_MCFCRT_EXTERN_C_END

#endif
