// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_asm_fpu.h"

#undef expm1f
#undef expm1
#undef expm1l

static inline long double fpu_expm1(long double x){
	bool sign;
	const __MCFCRT_FpuExamine exam = __MCFCRT_fxam(&sign, x);
	if(exam == __MCFCRT_kFpuExamineNaN){
		return x;
	}
	if(exam == __MCFCRT_kFpuExamineZero){
		return x;
	}
	if(exam == __MCFCRT_kFpuExamineInfinity){
		if(sign){
			return __MCFCRT_fchs(1);
		}
		return x;
	}
	// e^x = 2^(x*log2(e))
	const long double xlog2e = x * __MCFCRT_fldl2e();
	const long double i = __MCFCRT_ftrunc(xlog2e), m = xlog2e - i;
	if(i == 0){
		return __MCFCRT_f2xm1(m);
	}
	return __MCFCRT_fscale(1, i) * (__MCFCRT_f2xm1(m) + 1) - 1;
}

float expm1f(float x){
	return (float)fpu_expm1(x);
}
double expm1(double x){
	return (double)fpu_expm1(x);
}
long double expm1l(long double x){
	return fpu_expm1(x);
}
