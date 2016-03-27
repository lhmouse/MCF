// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_math_asm.h"
#include "_constants.h"

float sinf(float x){
	register float ret;
	__asm__ __volatile__(
		"fld dword ptr[%1] \n"
		"fsin \n"
		"fstsw ax \n"
		"test ah, 4 \n"
		"jz 1f \n"
		"	fld tbyte ptr[%2] \n"
		"	fxch st(1) \n"
		"	2: \n"
		"		fprem \n"
		"		fstsw ax \n"
		"		test ah, 4 \n"
		"	jnz 2b \n"
		"	fstp st(1) \n"
		"	fsin \n"
		"1: \n"
		__MCFCRT_FLT_RET_ST("%1")
		: __MCFCRT_FLT_RET_CONS(ret)
		: "m"(x), "m"(__MCFCRT_kMath_Pos_1p61_Pi)
		: "ax"
	);
	return ret;
}

double sin(double x){
	register double ret;
	__asm__ __volatile__(
		"fld qword ptr[%1] \n"
		"fsin \n"
		"fstsw ax \n"
		"test ah, 4 \n"
		"jz 1f \n"
		"	fld tbyte ptr[%2] \n"
		"	fxch st(1) \n"
		"	2: \n"
		"		fprem \n"
		"		fstsw ax \n"
		"		test ah, 4 \n"
		"	jnz 2b \n"
		"	fstp st(1) \n"
		"	fsin \n"
		"1: \n"
		__MCFCRT_DBL_RET_ST("%1")
		: __MCFCRT_DBL_RET_CONS(ret)
		: "m"(x), "m"(__MCFCRT_kMath_Pos_1p61_Pi)
		: "ax"
	);
	return ret;
}

long double sinl(long double x){
	register long double ret;
	__asm__ __volatile__(
		"fld tbyte ptr[%1] \n"
		"fsin \n"
		"fstsw ax \n"
		"test ah, 4 \n"
		"jz 1f \n"
		"	fld tbyte ptr[%2] \n"
		"	fxch st(1) \n"
		"	2: \n"
		"		fprem \n"
		"		fstsw ax \n"
		"		test ah, 4 \n"
		"	jnz 2b \n"
		"	fstp st(1) \n"
		"	fsin \n"
		"1: \n"
		__MCFCRT_LDBL_RET_ST("%1")
		: __MCFCRT_LDBL_RET_CONS(ret)
		: "m"(x), "m"(__MCFCRT_kMath_Pos_1p61_Pi)
		: "ax"
	);
	return ret;
}
