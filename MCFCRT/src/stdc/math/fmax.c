// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

#undef fmaxf
#undef fmax
#undef fmaxl

static inline long double fpu_fmax(long double x, long double y){
	long double ret;
	__asm__(
		"fcom st(1) \n"
		"fstsw ax \n"
		"test ah, 0x41 \n"
		"jnz 1f \n"
		"	fxch st(1) \n"
		"1: \n"
		"fstp st \n"
		: "=&t"(ret)
		: "0"(x), "u"(y)
		: "st(1)"
	);
	return ret;
}

float fmaxf(float x, float y){
#ifdef _WIN64
	float ret;
	__asm__(
		"movaps xmm2, xmm0 \n"
		"cmpltps xmm0, xmm1 \n"
		"xorps xmm1, xmm2 \n"
		"andps xmm0, xmm1 \n"
		"xorps xmm0, xmm2 \n"
		: "=Yz"(ret)
		: "0"(x), "x"(y)
		: "xmm2"
	);
	return ret;
#else
	return (float)fpu_fmax(x, y);
#endif
}
double fmax(double x, double y){
#ifdef _WIN64
	double ret;
	__asm__(
		"movapd xmm2, xmm0 \n"
		"cmpltpd xmm0, xmm1 \n"
		"xorpd xmm1, xmm2 \n"
		"andpd xmm0, xmm1 \n"
		"xorpd xmm0, xmm2 \n"
		: "=Yz"(ret)
		: "0"(x), "x"(y)
		: "xmm2"
	);
	return ret;
#else
	return (double)fpu_fmax(x, y);
#endif
}
long double fmaxl(long double x, long double y){
	return fpu_fmax(x, y);
}
