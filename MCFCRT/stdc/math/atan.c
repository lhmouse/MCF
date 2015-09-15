// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_math_asm.h"

float atanf(float x){
	register float ret;
	__asm__ __volatile__(
		"fld dword ptr[%1] \n"
		"fld1 \n"
		"fpatan \n"
		__MCF_FLT_RET_ST("%1")
		: __MCF_FLT_RET_CONS(ret)
		: "m"(x)
	);
	return ret;
}

double atan(double x){
	register double ret;
	__asm__ __volatile__(
		"fld qword ptr[%1] \n"
		"fld1 \n"
		"fpatan \n"
		__MCF_DBL_RET_ST("%1")
		: __MCF_DBL_RET_CONS(ret)
		: "m"(x)
	);
	return ret;
}

long double atanl(long double x){
	register long double ret;
	__asm__ __volatile__(
		"fld tbyte ptr[%1] \n"
		"fld1 \n"
		"fpatan \n"
		__MCF_LDBL_RET_ST("%1")
		: __MCF_LDBL_RET_CONS(ret)
		: "m"(x)
	);
	return ret;
}
