// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_STDC_MATH_FPU_H_
#define __MCFCRT_STDC_MATH_FPU_H_

#include "../../env/_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

// https://en.wikipedia.org/wiki/Extended_precision#x86_Extended_Precision_Format
typedef union __MCFCRT_tag_x87Register {
	struct __attribute__((__packed__)) {
		__extension__ union {
			_MCFCRT_STD uint64_t __mts_q;
			struct {
				_MCFCRT_STD uint32_t __mts_l;
				_MCFCRT_STD uint32_t __mts_h;
			};
		};
		_MCFCRT_STD uint16_t __exp : 15;
		_MCFCRT_STD uint16_t __sign : 1;
	};
	long double __val;
} __MCFCRT_x87Register;

typedef enum __MCFCRT_tagFpuCompare {
	__MCFCRT_kFpuComparePositive  = 0x0000,
	__MCFCRT_kFpuCompareNegative  = 0x0100,
	__MCFCRT_kFpuCompareZero      = 0x4000,
	__MCFCRT_kFpuCompareUnordered = 0x4100,
} __MCFCRT_FpuCompare;

static inline __MCFCRT_FpuCompare __MCFCRT_ftest(long double __x) _MCFCRT_NOEXCEPT {
	_MCFCRT_STD uint16_t __fsw;
	__asm__ (
		"ftst \n"
		"fstsw ax \n"
		: "=a"(__fsw)
		: "t"(__x)
	);
	return (__MCFCRT_FpuCompare)(__fsw & 0x4100);
}

typedef enum __MCFCRT_tagFpuExamine {
	__MCFCRT_kFpuExamineUnsupported = 0x0000,
	__MCFCRT_kFpuExamineNaN         = 0x0100,
	__MCFCRT_kFpuExamineNormal      = 0x0400,
	__MCFCRT_kFpuExamineInfinity    = 0x0500,
	__MCFCRT_kFpuExamineZero        = 0x4000,
	__MCFCRT_kFpuExamineEmpty       = 0x4100,
	__MCFCRT_kFpuExamineDenormal    = 0x4400,
	__MCFCRT_kFpuExamineReserved    = 0x4500,
} __MCFCRT_FpuExamine;

static inline __MCFCRT_FpuExamine __MCFCRT_fxam(bool *__sign, long double __x) _MCFCRT_NOEXCEPT {
	_MCFCRT_STD uint16_t __fsw;
	__asm__ (
		"fxam \n"
		"fstsw ax \n"
		: "=a"(__fsw)
		: "t"(__x)
	);
	*__sign = __fsw & 0x0200;
	return (__MCFCRT_FpuExamine)(__fsw & 0x4500);
}

static inline long double __MCFCRT_fabs(long double __x) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__ (
		"fabs \n"
		: "=&t"(__ret)
		: "0"(__x)
	);
	return __ret;
}
static inline long double __MCFCRT_fchs(long double __x) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__ (
		"fchs \n"
		: "=&t"(__ret)
		: "0"(__x)
	);
	return __ret;
}

static inline void __MCFCRT_fistp(int *__p, long double __x) _MCFCRT_NOEXCEPT {
	__asm__ (
		"fistp dword ptr[%0] \n"
		: "=m"(*__p)
		: "t"(__x)
		: "st"
	);
}
static inline void __MCFCRT_fistpl(long *__p, long double __x) _MCFCRT_NOEXCEPT {
	__asm__ (
		"fistp dword ptr[%0] \n"
		: "=m"(*__p)
		: "t"(__x)
		: "st"
	);
}
static inline void __MCFCRT_fistpll(long long *__p, long double __x) _MCFCRT_NOEXCEPT {
	__asm__ (
		"fistp qword ptr[%0] \n"
		: "=m"(*__p)
		: "t"(__x)
		: "st"
	);
}
/* TODO: SSE3
static inline void __MCFCRT_fisttp(int *__p, long double __x) _MCFCRT_NOEXCEPT {
	__asm__ (
		"fisttp dword ptr[%0] \n"
		: "=m"(*__p)
		: "t"(__x)
		: "st"
	);
}
static inline void __MCFCRT_fisttpl(long *__p, long double __x) _MCFCRT_NOEXCEPT {
	__asm__ (
		"fisttp dword ptr[%0] \n"
		: "=m"(*__p)
		: "t"(__x)
		: "st"
	);
}
static inline void __MCFCRT_fisttpll(long long *__p, long double __x) _MCFCRT_NOEXCEPT {
	__asm__ (
		"fisttp qword ptr[%0] \n"
		: "=m"(*__p)
		: "t"(__x)
		: "st"
	);
}
*/
static inline long double __MCFCRT_frndintany(long double __x) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__ (
		"frndint \n"
		: "=&t"(__ret)
		: "0"(__x)
	);
	return __ret;
}
static inline long double __MCFCRT_ftrunc(long double __x) _MCFCRT_NOEXCEPT {
	long double __ret;
	_MCFCRT_STD uint16_t __fcw;
	__asm__ (
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
	__asm__ (
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
	__asm__ (
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

static inline long double __MCFCRT_fsquare(long double __x) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__ (
		"fmul st \n"
		: "=&t"(__ret)
		: "0"(__x)
	);
	return __ret;
}
static inline long double __MCFCRT_fsqrt(long double __x) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__ (
		"fsqrt \n"
		: "=&t"(__ret)
		: "0"(__x)
	);
	return __ret;
}

static inline long double __MCFCRT_fmod(bool (*__bits)[3], long double __x, long double __y) _MCFCRT_NOEXCEPT {
	long double __ret;
	_MCFCRT_STD uint16_t __fsw;
	__asm__ (
		"1: \n"
		"	fprem \n"
		"	fstsw ax \n"
		"	test ah, 0x04 \n"
		"	jnz 1b \n"
		: "=&t"(__ret), "=a"(__fsw)
		: "0"(__x), "u"(__y)
	);
	(*__bits)[2] = __fsw & 0x0100;
	(*__bits)[1] = __fsw & 0x4000;
	(*__bits)[0] = __fsw & 0x0200;
	return __ret;
}
static inline long double __MCFCRT_fremainder(bool (*__bits)[3], long double __x, long double __y) _MCFCRT_NOEXCEPT {
	long double __ret;
	_MCFCRT_STD uint16_t __fsw;
	__asm__ (
		"1: \n"
		"	fprem1 \n"
		"	fstsw ax \n"
		"	test ah, 0x04 \n"
		"	jnz 1b \n"
		: "=&t"(__ret), "=a"(__fsw)
		: "0"(__x), "u"(__y)
	);
	(*__bits)[2] = __fsw & 0x0100;
	(*__bits)[1] = __fsw & 0x4000;
	(*__bits)[0] = __fsw & 0x0200;
	return __ret;
}

static inline long double __MCFCRT_fsin_unsafe(long double __x) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__ (
		"fsin \n"
		: "=&t"(__ret)
		: "0"(__x)
	);
	return __ret;
}
static inline long double __MCFCRT_fcos_unsafe(long double __x) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__ (
		"fcos \n"
		: "=&t"(__ret)
		: "0"(__x)
	);
	return __ret;
}
static inline void __MCFCRT_fsincos_unsafe(long double *__sinx, long double *__cosx, long double __x) _MCFCRT_NOEXCEPT {
	__asm__ (
		"fsincos \n"
		: "=&t"(*__cosx), "=&u"(*__sinx)
		: "0"(__x)
	);
}
static inline long double __MCFCRT_ftan_unsafe(long double __x) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__ (
		"fptan \n"
		"fstp st \n"
		: "=&t"(__ret)
		: "0"(__x)
	);
	return __ret;
}

static inline long double __MCFCRT_fpatan(long double __y, long double __x) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__ (
		"fpatan \n"
		: "=&t"(__ret)
		: "0"(__x), "u"(__y)
		: "st(1)"
	);
	return __ret;
}

static inline long double __MCFCRT_fxtract(long double *__pn, long double __x) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__ (
		"fxtract \n"
		: "=&t"(__ret), "=&u"(*__pn)
		: "0"(__x)
	);
	return __ret;
}

static inline long double __MCFCRT_fscale(long double __x, long double __n) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__ (
		"fscale \n"
		: "=&t"(__ret)
		: "0"(__x), "u"(__n)
	);
	return __ret;
}
static inline long double __MCFCRT_fyl2x(long double __y, long double __x) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__ (
		"fyl2x \n"
		: "=&t"(__ret)
		: "0"(__x), "u"(__y)
		: "st(1)"
	);
	return __ret;
}

static inline long double __MCFCRT_f2xm1(long double __x) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__ (
		"f2xm1 \n"
		: "=&t"(__ret)
		: "0"(__x)
	);
	return __ret;
}
static inline long double __MCFCRT_fyl2xp1(long double __y, long double __x) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__ (
		"fyl2xp1 \n"
		: "=&t"(__ret)
		: "0"(__x), "u"(__y)
		: "st(1)"
	);
	return __ret;
}

__attribute__((__const__))
static inline long double __MCFCRT_fldz(void) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__ (
		"fldz \n"
		: "=&t"(__ret)
	);
	return __ret;
}
__attribute__((__const__))
static inline long double __MCFCRT_fld1(void) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__ (
		"fld1 \n"
		: "=&t"(__ret)
	);
	return __ret;
}
__attribute__((__const__))
static inline long double __MCFCRT_fldl2e(void) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__ (
		"fldl2e \n"
		: "=&t"(__ret)
	);
	return __ret;
}
__attribute__((__const__))
static inline long double __MCFCRT_fldl2t(void) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__ (
		"fldl2t \n"
		: "=&t"(__ret)
	);
	return __ret;
}
__attribute__((__const__))
static inline long double __MCFCRT_fldlg2(void) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__ (
		"fldlg2 \n"
		: "=&t"(__ret)
	);
	return __ret;
}
__attribute__((__const__))
static inline long double __MCFCRT_fldln2(void) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__ (
		"fldln2 \n"
		: "=&t"(__ret)
	);
	return __ret;
}
__attribute__((__const__))
static inline long double __MCFCRT_fldpi(void) _MCFCRT_NOEXCEPT {
	long double __ret;
	__asm__ (
		"fldpi \n"
		: "=&t"(__ret)
	);
	return __ret;
}

_MCFCRT_EXTERN_C_END

#endif
