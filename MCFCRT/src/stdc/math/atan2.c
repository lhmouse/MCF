// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_asm_fpu.h"

#undef atan2f
#undef atan2
#undef atan2l

static inline long double fpu_atan2(long double y, long double x){
	bool ysign;
	const __MCFCRT_FpuExamine yexam = __MCFCRT_fxam(&ysign, y);
	if(yexam == __MCFCRT_kFpuExamineNaN){
		return y;
	}
	bool xsign;
	const __MCFCRT_FpuExamine xexam = __MCFCRT_fxam(&xsign, x);
	if(xexam == __MCFCRT_kFpuExamineNaN){
		return x;
	}
	return __MCFCRT_fpatan(y, x);
}

float atan2f(float y, float x){
	return (float)fpu_atan2(y, x);
}
double atan2(double y, double x){
	return (double)fpu_atan2(y, x);
}
long double atan2l(long double y, long double x){
	return fpu_atan2(y, x);
}
