// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_mathasm.h"

float log10f(float x){
	register float ret;
	__asm__ __volatile__(
		"fldlg2 \n"
		"fld dword ptr[%1] \n"
		"fyl2x \n"
		__FLT_RET_ST("%1")
		: __FLT_RET_CONS(ret)
		: "m"(x)
	);
	return ret;
}

double log10(double x){
	register double ret;
	__asm__ __volatile__(
		"fldlg2 \n"
		"fld qword ptr[%1] \n"
		"fyl2x \n"
		__DBL_RET_ST("%1")
		: __DBL_RET_CONS(ret)
		: "m"(x)
	);
	return ret;
}

__LDBL_DECL(log10l, long double x){
	register long double ret;
	__asm__ __volatile__(
		"fldlg2 \n"
		"fld tbyte ptr[%1] \n"
		"fyl2x \n"
		__LDBL_RET_ST()
		: __LDBL_RET_CONS(ret)
		: "m"(x), __LDBL_RET_CONS_IN()
		: "ax"
	);
	__LDBL_RETURN(ret);
}
