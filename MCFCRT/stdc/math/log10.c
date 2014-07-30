// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_mathasm.h"

float log10f(float x){
	register float ret;
	__asm__ __volatile__(
		"fldlg2 \n"
		"fld dword ptr[%1] \n"
		"fyl2x \n"
		__MCF_FLT_RET_ST("%1")
		: __MCF_FLT_RET_CONS(ret)
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
		__MCF_DBL_RET_ST("%1")
		: __MCF_DBL_RET_CONS(ret)
		: "m"(x)
	);
	return ret;
}

__MCF_LDBL_DECL(log10l, long double x){
	register long double ret;
	__asm__ __volatile__(
		"fldlg2 \n"
		"fld tbyte ptr[%1] \n"
		"fyl2x \n"
		__MCF_LDBL_RET_ST()
		: __MCF_LDBL_RET_CONS(ret)
		: "m"(x), __MCF_LDBL_RET_CONS_IN()
		: "ax"
	);
	__MCF_LDBL_RETURN(ret);
}
