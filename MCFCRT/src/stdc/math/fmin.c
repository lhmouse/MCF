// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

#undef fminf
#undef fmin
#undef fminl

static inline long double fpu_fmin(long double x, long double y){
	long double ret;
	__asm__(
		"fcomi st(1) \n"
		"fcmova st, st(1) \n"
		"fstp st(1) \n"
		: "=&t"(ret)
		: "0"(x), "u"(y)
		: "st(1)"
	);
	return ret;
}

float fminf(float x, float y){
#ifdef _WIN64
	float ret;
	__asm__(
		"movaps xmm2, xmm0 \n"
		"cmpltps xmm2, xmm1 \n"
		"xorps xmm0, xmm1 \n"
		"andps xmm0, xmm2 \n"
		"xorps xmm0, xmm1 \n"
		: "=Yz"(ret)
		: "0"(x), "x"(y)
		: "xmm2"
	);
	return ret;
#else
	return (float)fpu_fmin(x, y);
#endif
}
double fmin(double x, double y){
#ifdef _WIN64
	double ret;
	__asm__(
		"movapd xmm2, xmm0 \n"
		"cmpltpd xmm2, xmm1 \n"
		"xorpd xmm0, xmm1 \n"
		"andpd xmm0, xmm2 \n"
		"xorpd xmm0, xmm1 \n"
		: "=Yz"(ret)
		: "0"(x), "x"(y)
		: "xmm2"
	);
	return ret;
#else
	return (double)fpu_fmin(x, y);
#endif
}
long double fminl(long double x, long double y){
	return fpu_fmin(x, y);
}
