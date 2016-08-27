// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_asm.h"

float remainderf(float x, float y){
	register float ret;
	__asm__ volatile (
		"fld dword ptr[%2] \n"
		"fld dword ptr[%1] \n"
		"1: \n"
		"	fprem1 \n"
		"	fstsw ax \n"
		"	test ah, 4 \n"
		"jnz 1b \n"
		"fstp st(1) \n"
		__MCFCRT_FLT_RET_ST("%1")
		: __MCFCRT_FLT_RET_CONS(ret)
		: "m"(x), "m"(y)
		: "ax"
	);
	return ret;
}

double remainder(double x, double y){
	register double ret;
	__asm__ volatile (
		"fld qword ptr[%2] \n"
		"fld qword ptr[%1] \n"
		"1: \n"
		"	fprem1 \n"
		"	fstsw ax \n"
		"	test ah, 4 \n"
		"jnz 1b \n"
		"fstp st(1) \n"
		__MCFCRT_DBL_RET_ST("%1")
		: __MCFCRT_DBL_RET_CONS(ret)
		: "m"(x), "m"(y)
		: "ax"
	);
	return ret;
}

long double remainderl(long double x, long double y){
	register long double ret;
	__asm__ volatile (
		"fld tbyte ptr[%2] \n"
		"fld tbyte ptr[%1] \n"
		"1: \n"
		"	fprem1 \n"
		"	fstsw ax \n"
		"	test ah, 4 \n"
		"jnz 1b \n"
		"fstp st(1) \n"
		__MCFCRT_LDBL_RET_ST("%1")
		: __MCFCRT_LDBL_RET_CONS(ret)
		: "m"(x), "m"(y)
		: "ax"
	);
	return ret;
}
