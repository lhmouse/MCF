// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_asm_fpu.h"

#undef truncf
#undef trunc
#undef truncl

static inline long double fpu_trunc(long double x){
	bool sign;
	const __MCFCRT_FpuExamine exam = __MCFCRT_fxam(&sign, x);
	if(exam == __MCFCRT_kFpuExamineNaN){
		return x;
	}
	return __MCFCRT_ftrunc(x);
}

float truncf(float x){
	return (float)fpu_trunc(x);
}
double trunc(double x){
	return (double)fpu_trunc(x);
}
long double truncl(long double x){
	return fpu_trunc(x);
}
