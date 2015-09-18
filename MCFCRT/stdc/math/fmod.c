// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_math_asm.h"

float fmodf(float x, float y){
	register float ret;
	__asm__ __volatile__(
		"fld dword ptr[%2] \n"
		"fld dword ptr[%1] \n"
		"1: \n"
		"	fprem \n"
		"	fstsw ax \n"
		"	test ah, 4 \n"
		"jnz 1b \n"
		"fstp st(1) \n"
		__FLT_RET_ST("%1")
		: __FLT_RET_CONS(ret)
		: "m"(x), "m"(y)
		: "ax"
	);
	return ret;
}

double fmod(double x, double y){
	register double ret;
	__asm__ __volatile__(
		"fld qword ptr[%2] \n"
		"fld qword ptr[%1] \n"
		"1: \n"
		"	fprem \n"
		"	fstsw ax \n"
		"	test ah, 4 \n"
		"jnz 1b \n"
		"fstp st(1) \n"
		__DBL_RET_ST("%1")
		: __DBL_RET_CONS(ret)
		: "m"(x), "m"(y)
		: "ax"
	);
	return ret;
}

long double fmodl(long double x, long double y){
	register long double ret;
	__asm__ __volatile__(
		"fld tbyte ptr[%2] \n"
		"fld tbyte ptr[%1] \n"
		"1: \n"
		"	fprem \n"
		"	fstsw ax \n"
		"	test ah, 4 \n"
		"jnz 1b \n"
		"fstp st(1) \n"
		__LDBL_RET_ST("%1")
		: __LDBL_RET_CONS(ret)
		: "m"(x), "m"(y)
		: "ax"
	);
	return ret;
}
