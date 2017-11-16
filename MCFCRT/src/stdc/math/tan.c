// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_asm_fpu.h"

#undef tanf
#undef tan
#undef tanl

static inline long double fpu_tan(long double x){
	bool sign;
	const __MCFCRT_FpuExamine exam = __MCFCRT_fxam(&sign, x);
	if(exam == __MCFCRT_kFpuExamineNaN){
		return x;
	}
	bool bits[3];
	const long double reduced = __MCFCRT_fremainder(&bits, x, __MCFCRT_fldpi());
	long double ret = __MCFCRT_ftan_unsafe(reduced);
	return ret;
}

float tanf(float x){
	return (float)fpu_tan(x);
}
double tan(double x){
	return (double)fpu_tan(x);
}
long double tanl(long double x){
	return fpu_tan(x);
}
