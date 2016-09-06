// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

#undef fdimf
#undef fdim
#undef fdiml

static inline long double fpu_fdim(long double x, long double y){
	long double ret;
	__asm__(
		"fcom st(1) \n"
		"fstsw ax \n"
		"test ah, 0x41 \n"
		"fldz \n"
		"fxch st(2) \n"
		"jnz 1f \n"
		"	fsubr st(1) \n"
		"	fxch st(2) \n"
		"1: \n"
		"fstp st \n"
		"fstp st \n"
		: "=&t"(ret)
		: "0"(x), "u"(y)
		: "st(1)"
	);
	return ret;
}

float fdimf(float x, float y){
#ifdef _WIN64
	float ret;
	__asm__(
		"movaps xmm2, xmm1 \n"
		"cmpltps xmm1, xmm0 \n"
		"andps xmm0, xmm1 \n"
		"andps xmm2, xmm1 \n"
		"subps xmm0, xmm2 \n"
		: "=Yz"(ret)
		: "0"(x), "x"(y)
		: "xmm2"
	);
	return ret;
#else
	return (float)fpu_fdim(x, y);
#endif
}
double fdim(double x, double y){
#ifdef _WIN64
	double ret;
	__asm__(
		"movapd xmm2, xmm1 \n"
		"cmpltpd xmm1, xmm0 \n"
		"andpd xmm0, xmm1 \n"
		"andpd xmm2, xmm1 \n"
		"subpd xmm0, xmm2 \n"
		: "=Yz"(ret)
		: "0"(x), "x"(y)
		: "xmm2"
	);
	return ret;
#else
	return (double)fpu_fdim(x, y);
#endif
}
long double fdiml(long double x, long double y){
	return fpu_fdim(x, y);
}
