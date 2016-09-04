// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_asm.h"

float scalbnf(float x, int n){
	register float ret;
	__asm__ volatile (
		"fild dword ptr[%2] \n"
		"fld dword ptr[%1] \n"
		"fscale \n"
		"fstp st(1) \n"
		__MCFCRT_FLT_RET_ST("%1")
		: __MCFCRT_FLT_RET_CONS(ret)
		: "m"(x), "m"(n)
	);
	return ret;
}

double scalbn(double x, int n){
	register double ret;
	__asm__ volatile (
		"fild dword ptr[%2] \n"
		"fld qword ptr[%1] \n"
		"fscale \n"
		"fstp st(1) \n"
		__MCFCRT_DBL_RET_ST("%1")
		: __MCFCRT_DBL_RET_CONS(ret)
		: "m"(x), "m"(n)
	);
	return ret;
}

long double scalbnl(long double x, int n){
	register long double ret;
	__asm__ volatile (
		"fild dword ptr[%2] \n"
		"fld tbyte ptr[%1] \n"
		"fscale \n"
		"fstp st(1) \n"
		__MCFCRT_LDBL_RET_ST("%1")
		: __MCFCRT_LDBL_RET_CONS(ret)
		: "m"(x), "m"(n)
	);
	return ret;
}
