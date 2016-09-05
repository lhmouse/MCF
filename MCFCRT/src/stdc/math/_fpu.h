// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_MATH_FPU_H_
#define __MCFCRT_MATH_FPU_H_

#include "../../env/_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

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

_MCFCRT_EXTERN_C_END

#endif
