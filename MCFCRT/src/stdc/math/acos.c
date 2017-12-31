// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_asm_fpu.h"

#undef acosf
#undef acos
#undef acosl

static inline long double fpu_acos(long double x){
	bool sign;
	const __MCFCRT_FpuExamine exam = __MCFCRT_fxam(&sign, x);
	if(exam == __MCFCRT_kFpuExamineNaN){
		return x;
	}
	return __MCFCRT_fpatan(__MCFCRT_fsqrt(1 - __MCFCRT_fsquare(x)), x);
}

float acosf(float x){
	return (float)fpu_acos(x);
}
double acos(double x){
	return (double)fpu_acos(x);
}
long double acosl(long double x){
	return fpu_acos(x);
}
