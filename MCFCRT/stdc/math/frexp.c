// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_math_asm.h"
#include "_constants.h"

float frexpf(float x, int *exp){
	register float ret;
	__asm__ __volatile__(
		"fld dword ptr[%1] \n"
		"ftst \n"
		"fstsw ax \n"
		"test ah, 0x40 \n"
		"jz 1f \n"
		"	fldz \n"
		"	jmp 2f \n"
		"1: \n"
		"fxtract \n"
		"fmul qword ptr[%3] \n"
		"fld1 \n"
		"faddp st(2), st \n"
		"fxch st(1) \n"
		"2: \n"
		"fistp dword ptr[%2] \n"
		__FLT_RET_ST("%1")
		: __FLT_RET_CONS(ret)
		: "m"(x), "r"(exp), "m"(__MCFCRT_kMath_Pos_0_5)
		: "ax"
	);
	return ret;
}

double frexp(double x, int *exp){
	register double ret;
	__asm__ __volatile__(
		"fld qword ptr[%1] \n"
		"ftst \n"
		"fstsw ax \n"
		"test ah, 0x40 \n"
		"jz 1f \n"
		"	fldz \n"
		"	jmp 2f \n"
		"1: \n"
		"fxtract \n"
		"fmul qword ptr[%3] \n"
		"fld1 \n"
		"faddp st(2), st \n"
		"fxch st(1) \n"
		"2: \n"
		"fistp dword ptr[%2] \n"
		__DBL_RET_ST("%1")
		: __DBL_RET_CONS(ret)
		: "m"(x), "r"(exp), "m"(__MCFCRT_kMath_Pos_0_5)
		: "ax"
	);
	return ret;
}

long double frexpl(long double x, int *exp){
	register long double ret;
	__asm__ __volatile__(
		"fld tbyte ptr[%1] \n"
		"ftst \n"
		"fstsw ax \n"
		"test ah, 0x40 \n"
		"jz 1f \n"
		"	fldz \n"
		"	jmp 2f \n"
		"1: \n"
		"fxtract \n"
		"fmul qword ptr[%3] \n"
		"fld1 \n"
		"faddp st(2), st \n"
		"fxch st(1) \n"
		"2: \n"
		"fistp dword ptr[%2] \n"
		__LDBL_RET_ST("%1")
		: __LDBL_RET_CONS(ret)
		: "m"(x), "r"(exp), "m"(__MCFCRT_kMath_Pos_0_5)
		: "ax"
	);
	return ret;
}
