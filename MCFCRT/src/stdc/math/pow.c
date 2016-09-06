// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef powf
#undef pow
#undef powl

static inline long double fpu_pow(long double x, long double y){
	if(y == 0){
		return 1;
	}
	if(y == 1){
		return x;
	}
	if(x == 0){
		return 0;
	}
	// x^y = 2^(y*log2(x))
	long double ylog2x;
	bool neg;
	if(x > 0){
		ylog2x = y * __MCFCRT_fyl2x(1.0l, x);
		neg = false;
	} else {
		if(__MCFCRT_frndintany(y) != y){
			__builtin_trap();
		}
		ylog2x = y * __MCFCRT_fyl2x(1.0l, -x);
		int fsw;
		neg = __MCFCRT_fmod(&fsw, y, 2.0l) != 0;
	}
	const long double i = __MCFCRT_frndintany(ylog2x), m = ylog2x - i;
	const long double pr = __MCFCRT_fscale(1.0l, i) * (__MCFCRT_f2xm1(m) + 1.0l);
	if(neg){
		return -pr;
	}
	return pr;
}

float powf(float x, float y){
	return (float)fpu_pow(x, y);
}
double pow(double x, double y){
	return (double)fpu_pow(x, y);
}
long double powl(long double x, long double y){
	return fpu_pow(x, y);
}
