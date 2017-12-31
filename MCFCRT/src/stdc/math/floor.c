// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_asm_fpu.h"

#undef floorf
#undef floor
#undef floorl

static inline long double fpu_floor(long double x){
	bool sign;
	const __MCFCRT_FpuExamine exam = __MCFCRT_fxam(&sign, x);
	if(exam == __MCFCRT_kFpuExamineNaN){
		return x;
	}
	return __MCFCRT_ffloor(x);
}

float floorf(float x){
	return (float)fpu_floor(x);
}
double floor(double x){
	return (double)fpu_floor(x);
}
long double floorl(long double x){
	return fpu_floor(x);
}
