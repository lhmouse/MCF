// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_mathasm.h"

float fdimf(float x, float y){
	register float ret;
	__asm__ __volatile__(
#ifdef _WIN64
		"movss xmm0, dword ptr[%1] \n"
		"subss xmm0, dword ptr[%2] \n"
		"xorps xmm1, xmm1 \n"
		"cmpss xmm1, xmm0, 1 \n"
		"andps xmm0, xmm1 \n"
#else
		"fld dword ptr[%1] \n"
		"fsub dword ptr[%2] \n"
		"ftst \n"
		"fstsw ax \n"
		"fstp dword ptr[%1] \n"
		"and ah, 0x41 \n"
		"neg ah \n"
		"sbb eax, eax \n"
		"not eax \n"
		"and dword ptr[%1], eax \n"
		"fld dword ptr[%1] \n"
#endif
		: __FLT_RET_CONS(ret)
		: "m"(x), "m"(y)
		: "ax"
	);
	return ret;
}

double fdim(double x, double y){
	register double ret;
	__asm__ __volatile__(
#ifdef _WIN64
		"movsd xmm0, qword ptr[%1] \n"
		"subsd xmm0, qword ptr[%2] \n"
		"xorpd xmm1, xmm1 \n"
		"cmpsd xmm1, xmm0, 1 \n"
		"andpd xmm0, xmm1 \n"
#else
		"fld qword ptr[%1] \n"
		"fsub qword ptr[%2] \n"
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
#endif
		: __DBL_RET_CONS(ret)
		: "m"(x), "m"(y)
		: "ax"
	);
	return ret;
}

__LDBL_DECL(fdiml, long double x, long double y){
	register long double ret;
	__asm__ __volatile__(
		"fld tbyte ptr[%1] \n"
		"fld tbyte ptr[%2] \n"
		"fsubp st(1), st \n"
		"ftst \n"
		"fstsw ax \n"
		"fstp tbyte ptr[%1] \n"
		"and ah, 0x41 \n"
		"neg ah \n"
#ifdef _WIN64
		"sbb rax, rax \n"
		"not rax \n"
		"and qword ptr[%1], rax \n"
#else
		"sbb eax, eax \n"
		"not eax \n"
		"and dword ptr[%1], eax \n"
		"and dword ptr[%1 + 4], eax \n"
#endif
		"and word ptr[%1 + 8], ax \n"
		__LDBL_RET_MEM("%1", "a")
		: __LDBL_RET_CONS(ret)
		: "m"(x), "m"(y), __LDBL_RET_CONS_IN()
		: "ax"
	);
	__LDBL_RETURN(ret);
}
