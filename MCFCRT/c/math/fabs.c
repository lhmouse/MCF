// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_mathasm.h"

float fabsf(float x){
	register float ret;
	__asm__ __volatile__(
		"and byte ptr[%1 + 3], 0x7F \n"
#ifdef _WIN64
		"movss xmm0, dword ptr[%1] \n"
#else
		"fld dword ptr[%1] \n"
#endif
		: __FLT_RET_CONS(ret)
		: "m"(x)
	);
	return ret;
}

double fabs(double x){
	register double ret;
	__asm__ __volatile__(
		"and byte ptr[%1 + 7], 0x7F \n"
#ifdef _WIN64
		"movsd xmm0, qword ptr[%1] \n"
#else
		"fld qword ptr[%1] \n"
#endif
		: __DBL_RET_CONS(ret)
		: "m"(x)
	);
	return ret;
}

__LDBL_DECL(fabsl, long double x){
	register long double ret;
	__asm__ __volatile__(
		"and byte ptr[%1 + 9], 0x7F \n"
#ifdef _WIN64
		"mov rdx, qword ptr[%1] \n"
		"mov qword ptr[%2], rdx \n"
		"movzx rdx, word ptr[%1 + 8] \n"
		"mov word ptr[%2 + 8], dx \n"
#else
		"fld tbyte ptr[%1] \n"
#endif
		: __LDBL_RET_CONS(ret)
		: "m"(x), __LDBL_RET_CONS_IN()
#ifdef _WIN64
		: "rdx"
#endif
	);
	__LDBL_RETURN(ret);
}
