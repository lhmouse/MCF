// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_asm_fpu.h"

#undef scalbnf
#undef scalbn
#undef scalbnl

static inline long double fpu_scalbn(long double x, int n){
	bool sign;
	const __MCFCRT_FpuExamine exam = __MCFCRT_fxam(&sign, x);
	if(exam == __MCFCRT_kFpuExamineNaN){
		return x;
	}
	return __MCFCRT_fscale(x, n);
}

float scalbnf(float x, int n){
	return (float)fpu_scalbn(x, n);
}
double scalbn(double x, int n){
	return (double)fpu_scalbn(x, n);
}
long double scalbnl(long double x, int n){
	return fpu_scalbn(x, n);
}
