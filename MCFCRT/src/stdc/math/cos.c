// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_asm_fpu.h"

#undef cosf
#undef cos
#undef cosl

static inline long double fpu_cos(long double x){
	bool sign;
	const __MCFCRT_FpuExamine exam = __MCFCRT_fxam(&sign, x);
	if(exam == __MCFCRT_kFpuExamineNaN){
		return x;
	}
	bool bits[3];
	const long double reduced = __MCFCRT_fremainder(&bits, x, __MCFCRT_fldpi());
	long double ret = __MCFCRT_fcos_unsafe(reduced);
	if(bits[0]){
		ret = __MCFCRT_fchs(ret);
	}
	return ret;
}

float cosf(float x){
	return (float)fpu_cos(x);
}
double cos(double x){
	return (double)fpu_cos(x);
}
long double cosl(long double x){
	return fpu_cos(x);
}
