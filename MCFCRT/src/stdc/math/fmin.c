// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef fminf
#undef fmin
#undef fminl

static inline long double fpu_fmin(long double x, long double y){
	bool xsign;
	const __MCFCRT_FpuExamine xexam = __MCFCRT_fxam(&xsign, x);
	if(xexam == __MCFCRT_kFpuExamineNaN){
		return y;
	}
	bool ysign;
	const __MCFCRT_FpuExamine yexam = __MCFCRT_fxam(&ysign, y);
	if(yexam == __MCFCRT_kFpuExamineNaN){
		return x;
	}
	return (x < y) ? x : y;
}

float fminf(float x, float y){
	return (float)fpu_fmin(x, y);
}
double fmin(double x, double y){
	return (double)fpu_fmin(x, y);
}
long double fminl(long double x, long double y){
	return fpu_fmin(x, y);
}
