// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

#undef fmaxf
#undef fmax
#undef fmaxl

static inline long double fpu_fmax(long double x, long double y){
	long double ret;
	__asm__ (
		"fcomi st(1) \n"
		"fcmovb st, st(1) \n"
		"fstp st(1) \n"
		: "=&t"(ret)
		: "0"(x), "u"(y)
		: "st(1)"
	);
	return ret;
}

float fmaxf(float x, float y){
#ifdef _WIN64
	float ret;
	__asm__ (
		"movaps xmm2, %2 \n"
		"cmpltps xmm2, xmm0 \n"
		"xorps xmm0, %2 \n"
		"andps xmm0, xmm2 \n"
		"xorps xmm0, %2 \n"
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
	__asm__ (
		"movapd xmm2, %2 \n"
		"cmpltpd xmm2, xmm0 \n"
		"xorpd xmm0, %2 \n"
		"andpd xmm0, xmm2 \n"
		"xorpd xmm0, %2 \n"
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
