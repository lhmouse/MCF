// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_asm.h"

float sqrtf(float x){
	register float ret;
	__asm__ volatile (
		"fld dword ptr[%1] \n"
		"fsqrt \n"
		__MCFCRT_FLT_RET_ST("%1")
		: __MCFCRT_FLT_RET_CONS(ret)
		: "m"(x)
	);
	return ret;
}

double sqrt(double x){
	register double ret;
	__asm__ volatile (
		"fld qword ptr[%1] \n"
		"fsqrt \n"
		__MCFCRT_DBL_RET_ST("%1")
		: __MCFCRT_DBL_RET_CONS(ret)
		: "m"(x)
	);
	return ret;
}

long double sqrtl(long double x){
	register long double ret;
	__asm__ volatile (
		"fld tbyte ptr[%1] \n"
		"fsqrt \n"
		__MCFCRT_LDBL_RET_ST("%1")
		: __MCFCRT_LDBL_RET_CONS(ret)
		: "m"(x)
	);
	return ret;
}
