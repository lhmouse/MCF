// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014 LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_mathasm.h"

float exp2f(float x){
	register float ret;
	__asm__ __volatile__(
		"fld dword ptr[%1] \n"
		"fld st \n"
		"frndint \n"
		"fsub st(1), st \n"
		"fld1 \n"
		"fscale \n"
		"fstp st(1) \n"
		"fxch st(1) \n"
		"f2xm1 \n"
		"fld1 \n"
		"faddp st(1), st \n"
		"fmulp st(1), st \n"
		__MCF_FLT_RET_ST("%1")
		: __MCF_FLT_RET_CONS(ret)
		: "m"(x)
	);
	return ret;
}

double exp2(double x){
	register double ret;
	__asm__ __volatile__(
		"fld qword ptr[%1] \n"
		"fld st \n"
		"frndint \n"
		"fsub st(1), st \n"
		"fld1 \n"
		"fscale \n"
		"fstp st(1) \n"
		"fxch st(1) \n"
		"f2xm1 \n"
		"fld1 \n"
		"faddp st(1), st \n"
		"fmulp st(1), st \n"
		__MCF_DBL_RET_ST("%1")
		: __MCF_DBL_RET_CONS(ret)
		: "m"(x)
	);
	return ret;
}

__MCF_LDBL_DECL(exp2l, long double x){
	register long double ret;
	__asm__ __volatile__(
		"fld tbyte ptr[%1] \n"
		"fld st \n"
		"frndint \n"
		"fsub st(1), st \n"
		"fld1 \n"
		"fscale \n"
		"fstp st(1) \n"
		"fxch st(1) \n"
		"f2xm1 \n"
		"fld1 \n"
		"faddp st(1), st \n"
		"fmulp st(1), st \n"
		__MCF_LDBL_RET_ST()
		: __MCF_LDBL_RET_CONS(ret)
		: "m"(x), __MCF_LDBL_RET_CONS_IN()
	);
	__MCF_LDBL_RETURN(ret);
}
