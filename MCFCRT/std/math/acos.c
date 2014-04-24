// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_mathasm.h"

float acosf(float x){
	register float ret;
	__asm__ __volatile__(
		"fld dword ptr[%1] \n"
		"fld st \n"
		"fmul st, st \n"
		"fld1\n"
		"fsubrp st(1), st \n"
		"fsqrt \n"
		"fxch st(1) \n"
		"fpatan \n"
		__FLT_RET_ST("%1")
		: __FLT_RET_CONS(ret)
		: "m"(x)
	);
	return ret;
}

double acos(double x){
	register double ret;
	__asm__ __volatile__(
		"fld qword ptr[%1] \n"
		"fld st \n"
		"fmul st, st \n"
		"fld1\n"
		"fsubrp st(1), st \n"
		"fsqrt \n"
		"fxch st(1) \n"
		"fpatan \n"
		__DBL_RET_ST("%1")
		: __DBL_RET_CONS(ret)
		: "m"(x)
	);
	return ret;
}

__LDBL_DECL(acosl, long double x){
	register long double ret;
	__asm__ __volatile__(
		"fld tbyte ptr[%1] \n"
		"fld st \n"
		"fmul st, st \n"
		"fld1\n"
		"fsubrp st(1), st \n"
		"fsqrt \n"
		"fxch st(1) \n"
		"fpatan \n"
		__LDBL_RET_ST()
		: __LDBL_RET_CONS(ret)
		: "m"(x), __LDBL_RET_CONS_IN()
	);
	__LDBL_RETURN(ret);
}
