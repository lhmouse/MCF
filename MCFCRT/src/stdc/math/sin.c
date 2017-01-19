// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef sinf
#undef sin
#undef sinl

static inline long double fpu_sin(long double x){
	bool sign;
	const __MCFCRT_FpuExamine exam = __MCFCRT_fxam(&sign, x);
	if(exam == __MCFCRT_kFpuExamineNaN){
		return x;
	}
	bool bits[3];
	const long double reduced = __MCFCRT_fremainder(&bits, x, __MCFCRT_fldpi());
	long double ret = __MCFCRT_fsin_unsafe(reduced);
	if(bits[0]){
		ret = __MCFCRT_fchs(ret);
	}
	return ret;
}

float sinf(float x){
	return (float)fpu_sin(x);
}
double sin(double x){
	return (double)fpu_sin(x);
}
long double sinl(long double x){
	return fpu_sin(x);
}
