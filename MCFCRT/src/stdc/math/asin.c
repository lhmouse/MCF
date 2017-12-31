// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_asm_fpu.h"

#undef asinf
#undef asin
#undef asinl

static inline long double fpu_asin(long double x){
	bool sign;
	const __MCFCRT_FpuExamine exam = __MCFCRT_fxam(&sign, x);
	if(exam == __MCFCRT_kFpuExamineNaN){
		return x;
	}
	return __MCFCRT_fpatan(x, __MCFCRT_fsqrt(1 - __MCFCRT_fsquare(x)));
}

float asinf(float x){
	return (float)fpu_asin(x);
}
double asin(double x){
	return (double)fpu_asin(x);
}
long double asinl(long double x){
	return fpu_asin(x);
}
