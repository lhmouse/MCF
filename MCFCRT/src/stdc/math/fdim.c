// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

#undef fdimf
#undef fdim
#undef fdiml

static inline long double fpu_fdim(long double x, long double y){
	long double ret;
	__asm__(
		"fcomi st(1) \n"
		"fldz \n"
		"fcmova st, st(2) \n"
		"fldz \n"
		"fcmova st, st(2) \n"
		"fsubrp st(1), st \n"
		"fxch st(2) \n"
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
	float ret, unused;
	__asm__(
		"movaps xmm2, %3 \n"
		"cmpltps xmm2, xmm0 \n"
		"andps xmm0, xmm2 \n"
		"andps %3, xmm2 \n"
		"subps xmm0, %3 \n"
		: "=Yz"(ret), "=x"(unused)
		: "0"(x), "1"(y)
		: "xmm2"
	);
	return ret;
#else
	return (float)fpu_fdim(x, y);
#endif
}
double fdim(double x, double y){
#ifdef _WIN64
	double ret, unused;
	__asm__(
		"movapd xmm2, %3 \n"
		"cmpltpd xmm2, xmm0 \n"
		"andpd xmm0, xmm2 \n"
		"andpd %3, xmm2 \n"
		"subpd xmm0, %3 \n"
		: "=Yz"(ret), "=x"(unused)
		: "0"(x), "1"(y)
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
