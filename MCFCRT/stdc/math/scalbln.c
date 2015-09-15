// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_math_asm.h"

float scalblnf(float x, long n){
	register float ret;
	__asm__ __volatile__(
		"fild dword ptr[%2] \n"
		"fld dword ptr[%1] \n"
		"fscale \n"
		"fstp st(1) \n"
		__MCF_FLT_RET_ST("%1")
		: __MCF_FLT_RET_CONS(ret)
		: "m"(x), "m"(n)
	);
	return ret;
}

double scalbln(double x, long n){
	register double ret;
	__asm__ __volatile__(
		"fild dword ptr[%2] \n"
		"fld qword ptr[%1] \n"
		"fscale \n"
		"fstp st(1) \n"
		__MCF_DBL_RET_ST("%1")
		: __MCF_DBL_RET_CONS(ret)
		: "m"(x), "m"(n)
	);
	return ret;
}

long double scalblnl(long double x, long n){
	register long double ret;
	__asm__ __volatile__(
		"fild dword ptr[%2] \n"
		"fld tbyte ptr[%1] \n"
		"fscale \n"
		"fstp st(1) \n"
		__MCF_LDBL_RET_ST("%1")
		: __MCF_LDBL_RET_CONS(ret)
		: "m"(x), "m"(n)
	);
	return ret;
}
