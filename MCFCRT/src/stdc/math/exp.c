// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_asm_fpu.h"

#undef expf
#undef exp
#undef expl

static inline long double fpu_exp(long double x){
	bool sign;
	const __MCFCRT_FpuExamine exam = __MCFCRT_fxam(&sign, x);
	if(exam == __MCFCRT_kFpuExamineNaN){
		return x;
	}
	if(exam == __MCFCRT_kFpuExamineZero){
		return 1;
	}
	if(exam == __MCFCRT_kFpuExamineInfinity){
		if(sign){
			return __MCFCRT_fldz();
		}
		return x;
	}
	// e^x = 2^(x* log2(e))
	const long double xlog2e = x * __MCFCRT_fldl2e();
	const long double i = __MCFCRT_frndintany(xlog2e), m = xlog2e - i;
	return __MCFCRT_fscale(1, i) * (__MCFCRT_f2xm1(m) + 1);
}

float expf(float x){
	return (float)fpu_exp(x);
}
double exp(double x){
	return (double)fpu_exp(x);
}
long double expl(long double x){
	return fpu_exp(x);
}
