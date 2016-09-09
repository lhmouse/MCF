// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef cbrtf
#undef cbrt
#undef cbrtl

static inline long double fpu_cbrt(long double x){
	const __MCFCRT_FpuSign sgn = __MCFCRT_ftest(x);
	if(sgn == __MCFCRT_kFpuZero){
		return 0;
	} else {
		// x^(1/3) = 2^((1/3)*log2(x))
		long double ylog2x;
		bool neg;
		if(sgn == __MCFCRT_kFpuPositive){
			ylog2x = (1.0l / 3) * __MCFCRT_fyl2x(__MCFCRT_fld1(), x);
			neg = false;
		} else {
			ylog2x = (1.0l / 3) * __MCFCRT_fyl2x(__MCFCRT_fld1(), -x);
			neg = true;
		}
		const long double i = __MCFCRT_frndintany(ylog2x), m = ylog2x - i;
		long double ret = __MCFCRT_fscale(__MCFCRT_fld1(), i) * (__MCFCRT_f2xm1(m) + __MCFCRT_fld1());
		if(neg){
			ret = __MCFCRT_fneg(ret);
		}
		return ret;
	}
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
