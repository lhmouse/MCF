// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_asm_fpu.h"

#undef scalblnf
#undef scalbln
#undef scalblnl

static inline long double fpu_scalbln(long double x, long n){
	bool sign;
	const __MCFCRT_FpuExamine exam = __MCFCRT_fxam(&sign, x);
	if(exam == __MCFCRT_kFpuExamineNaN){
		return x;
	}
	return __MCFCRT_fscale(x, n);
}

float scalblnf(float x, long n){
	return (float)fpu_scalbln(x, n);
}
double scalbln(double x, long n){
	return (double)fpu_scalbln(x, n);
}
long double scalblnl(long double x, long n){
	return fpu_scalbln(x, n);
}
