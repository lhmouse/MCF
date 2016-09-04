// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_asm.h"
#include "_constants.h"

float fdimf(float x, float y){
	register float ret;
	__asm__ volatile (
#ifdef _WIN64
		"subss xmm0, %2 \n"
		"xorps xmm2, xmm2 \n"
		"cmpltss xmm2, xmm0 \n"
		"andps xmm0, xmm2 \n"
		: __MCFCRT_FLT_RET_CONS(ret)
		: "0"(x), "x"(y)
		: "ax", "xmm2"
#else
		"fld dword ptr[%1] \n"
		"fld dword ptr[%2] \n"
		"fsubp st(1), st \n"
		"ftst \n"
		"fstsw ax \n"
		"fstp dword ptr[%1] \n"
		"and ah, 0x41 \n"
		"neg ah \n"
		"sbb eax, eax \n"
		"not eax \n"
		"and dword ptr[%1], eax \n"
		"fld dword ptr[%1] \n"
		: __MCFCRT_FLT_RET_CONS(ret)
		: "m"(x), "m"(y)
		: "ax"
#endif
	);
	return ret;
}

double fdim(double x, double y){
	register double ret;
	__asm__ volatile (
#ifdef _WIN64
		"subsd xmm0, %2 \n"
		"xorpd xmm2, xmm2 \n"
		"cmpltsd xmm2, xmm0 \n"
		"andpd xmm0, xmm2 \n"
		: __MCFCRT_DBL_RET_CONS(ret)
		: "0"(x), "x"(y)
		: "ax", "xmm2"
#else
		"fld qword ptr[%1] \n"
		"fld qword ptr[%2] \n"
		"fsubp st(1), st \n"
		"ftst \n"
		"fstsw ax \n"
		"fstp qword ptr[%1] \n"
		"and ah, 0x41 \n"
		"neg ah \n"
		"sbb eax, eax \n"
		"not eax \n"
		"and dword ptr[%1], eax \n"
		"and dword ptr[%1 + 4], eax \n"
		"fld qword ptr[%1] \n"
		: __MCFCRT_DBL_RET_CONS(ret)
		: "m"(x), "m"(y)
		: "ax"
#endif
	);
	return ret;
}

long double fdiml(long double x, long double y){
	register long double ret;
	__asm__ volatile (
		"fld tbyte ptr[%1] \n"
		"fld tbyte ptr[%2] \n"
		"fsubp st(1), st \n"
		"ftst \n"
		"fstsw ax \n"
		"fstp tbyte ptr[%1] \n"
		"and ah, 0x41 \n"
		"neg ah \n"
		"sbb eax, eax \n"
		"not eax \n"
		"and dword ptr[%1], eax \n"
		"and dword ptr[%1 + 4], eax \n"
		"and word ptr[%1 + 8], ax \n"
		"fld tbyte ptr[%1] \n"
		: __MCFCRT_LDBL_RET_CONS(ret)
		: "m"(x), "m"(y)
		: "ax"
	);
	return ret;
}
