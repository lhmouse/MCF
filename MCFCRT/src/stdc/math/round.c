// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef roundf
#undef round
#undef roundl

static inline long double fpu_round(long double x){
	bool sign;
	const __MCFCRT_FpuExamine exam = __MCFCRT_fxam(&sign, x);
	if(exam == __MCFCRT_kFpuExamineNaN){
		return x;
	}
	return sign ? __MCFCRT_ftrunc(x - 0.5l)
	            : __MCFCRT_ftrunc(x + 0.5l);
}

float roundf(float x){
	return (float)fpu_round(x);
}
double round(double x){
	return (double)fpu_round(x);
}
long double roundl(long double x){
	return fpu_round(x);
}
