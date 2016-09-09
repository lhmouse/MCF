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

	const __MCFCRT_FpuSign sgn = __MCFCRT_ftest(x);
	if(sgn == __MCFCRT_kFpuZero){
		return 0;
	} else {
		// x^y = 2^(y*log2(x))
		long double ylog2x;
		bool neg;
		if(sgn == __MCFCRT_kFpuPositive){
			ylog2x = y * __MCFCRT_fyl2x(__MCFCRT_fld1(), x);
			neg = false;
		} else {
			if(__MCFCRT_frndintany(y) != y){
				return __builtin_nansl("0x706F77");
			}
			ylog2x = y * __MCFCRT_fyl2x(__MCFCRT_fld1(), -x);
			unsigned fsw;
			neg = __MCFCRT_fmod(&fsw, y, 2.0l) != 0;
		}
		const long double i = __MCFCRT_frndintany(ylog2x), m = ylog2x - i;
		long double ret = __MCFCRT_fscale(__MCFCRT_fld1(), i) * (__MCFCRT_f2xm1(m) + __MCFCRT_fld1());
		if(neg){
			ret = __MCFCRT_fneg(ret);
		}
		return ret;
	}
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
