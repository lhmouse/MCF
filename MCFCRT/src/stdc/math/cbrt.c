// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_asm_fpu.h"

#undef cbrtf
#undef cbrt
#undef cbrtl

static inline long double fpu_cbrt(long double x){
	bool xsign;
	const __MCFCRT_FpuExamine xexam = __MCFCRT_fxam(&xsign, x);
	if(xexam == __MCFCRT_kFpuExamineNaN){
		return x;
	}
	if(xexam == __MCFCRT_kFpuExamineZero){
		return x;
	}
	if(xexam == __MCFCRT_kFpuExamineInfinity){
		return x;
	}
	const long double xabs = __MCFCRT_fabs(x);
	// x^(1/3) = 2^(log2(x)/3)
	const long double ylog2x = __MCFCRT_fyl2x(1, xabs) / 3;
	const long double i = __MCFCRT_frndintany(ylog2x), m = ylog2x - i;
	long double ret = __MCFCRT_fscale(1, i) * (__MCFCRT_f2xm1(m) + 1);
	if(xsign){
		ret = __MCFCRT_fchs(ret);
	}
	return ret;
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
