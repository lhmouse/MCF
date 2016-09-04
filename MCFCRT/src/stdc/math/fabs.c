// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_asm.h"
#include "_constants.h"

float fabsf(float x){
	register float ret;
	__asm__ volatile (
#ifdef _WIN64
		"movss xmm1, dword ptr[%2] \n"
		"andps xmm0, xmm1 \n"
		: __MCFCRT_FLT_RET_CONS(ret)
		: "0"(x), "m"(__MCFCRT_kI32Max_p4)
		: "ax", "xmm1"
#else
		"mov eax, dword ptr[%1] \n"
		"shl eax, 1 \n"
		"shr eax, 1 \n"
		"mov dword ptr[%1], eax \n"
		"fld dword ptr[%1] \n"
		: __MCFCRT_FLT_RET_CONS(ret)
		: "m"(x)
		: "ax"
#endif
	);
	return ret;
}

double fabs(double x){
	register double ret;
	__asm__ volatile (
#ifdef _WIN64
		"movsd xmm1, qword ptr[%2] \n"
		"andpd xmm0, xmm1 \n"
		: __MCFCRT_DBL_RET_CONS(ret)
		: "0"(x), "m"(__MCFCRT_kI64Max_p2)
		: "ax", "xmm1"
#else
		"mov eax, dword ptr[%1 + 4] \n"
		"shl eax, 1 \n"
		"shr eax, 1 \n"
		"mov dword ptr[%1 + 4], eax \n"
		"fld qword ptr[%1] \n"
		: __MCFCRT_DBL_RET_CONS(ret)
		: "m"(x)
		: "ax"
#endif
	);
	return ret;
}

long double fabsl(long double x){
	register long double ret;
	__asm__ volatile (
		"movzx eax, word ptr[%1 + 8] \n"
		"shl eax, 17 \n"
		"shr eax, 17 \n"
		"mov word ptr[%1 + 8], ax \n"
		"fld tbyte ptr[%1] \n"
		: __MCFCRT_LDBL_RET_CONS(ret)
		: "m"(x)
		: "ax"
	);
	return ret;
}
