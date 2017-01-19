// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef logbf
#undef logb
#undef logbl

static inline long double fpu_logb(long double x){
	bool sign;
	const __MCFCRT_FpuExamine exam = __MCFCRT_fxam(&sign, x);
	if(exam == __MCFCRT_kFpuExamineNaN){
		return x;
	}
	long double ret;
	__MCFCRT_fxtract(&ret, x);
	return ret;
}

float logbf(float x){
	return (float)fpu_logb(x);
}
double logb(double x){
	return (double)fpu_logb(x);
}
long double logbl(long double x){
	return fpu_logb(x);
}
