// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef exp2f
#undef exp2
#undef exp2l

static inline long double fpu_exp2(long double x){
	const long double i = __MCFCRT_frndintany(x), m = x - i;
	return __MCFCRT_fscale(1.0l, i) * (__MCFCRT_f2xm1(m) + 1.0l);
}

float exp2f(float x){
	return (float)fpu_exp2(x);
}
double exp2(double x){
	return (double)fpu_exp2(x);
}
long double exp2l(long double x){
	return fpu_exp2(x);
}
