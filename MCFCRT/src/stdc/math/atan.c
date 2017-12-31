// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_asm_fpu.h"

#undef atanf
#undef atan
#undef atanl

static inline long double fpu_atan(long double x){
	bool sign;
	const __MCFCRT_FpuExamine exam = __MCFCRT_fxam(&sign, x);
	if(exam == __MCFCRT_kFpuExamineNaN){
		return x;
	}
	return __MCFCRT_fpatan(x, 1);
}

float atanf(float x){
	return (float)fpu_atan(x);
}
double atan(double x){
	return (double)fpu_atan(x);
}
long double atanl(long double x){
	return fpu_atan(x);
}
