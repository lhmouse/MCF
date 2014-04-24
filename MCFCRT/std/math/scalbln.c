// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_mathasm.h"

float scalblnf(float x, long n){
	register float ret;
	__asm__ __volatile__(
		"fild dword ptr[%2] \n"
		"fld dword ptr[%1] \n"
		"fscale \n"
		"fstp st(1) \n"
		__FLT_RET_ST("%1")
		: __FLT_RET_CONS(ret)
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
		__DBL_RET_ST("%1")
		: __DBL_RET_CONS(ret)
		: "m"(x), "m"(n)
	);
	return ret;
}

__LDBL_DECL(scalblnl, long double x, long n){
	register long double ret;
	__asm__ __volatile__(
		"fild dword ptr[%2] \n"
		"fld tbyte ptr[%1] \n"
		"fscale \n"
		"fstp st(1) \n"
		__LDBL_RET_ST()
		: __LDBL_RET_CONS(ret)
		: "m"(x), "m"(n), __LDBL_RET_CONS_IN()
	);
	__LDBL_RETURN(ret);
}
