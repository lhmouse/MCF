// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef cbrtf
#undef cbrt
#undef cbrtl

static inline long double fpu_cbrt(long double x){
	if(x == 0){
		return 0;
	}
	// x^(1/3) = 2^((1/3)*log2(x))
	long double ylog2x;
	bool neg;
	if(x > 0){
		ylog2x = 0.33333333333333333333333333333l * __MCFCRT_fyl2x(1.0l, x);
		neg = false;
	} else {
		ylog2x = 0.33333333333333333333333333333l * __MCFCRT_fyl2x(1.0l, -x);
		neg = true;
	}
	const long double i = __MCFCRT_frndintany(ylog2x), m = ylog2x - i;
	const long double pr = __MCFCRT_fscale(1.0l, i) * (__MCFCRT_f2xm1(m) + 1.0l);
	if(neg){
		return -pr;
	}
	return pr;
}

float cbrtf(float x){
	return (float)fpu_cbrt(x);
}
double cbrt(double x){
	return (double)fpu_cbrt(x);
}
long double cbrtl(long double x){
	return fpu_cbrt(x);
}
