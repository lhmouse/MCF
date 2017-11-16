// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_asm_fpu.h"
#include "_asm_sse2.h"

#undef fmaxf
#undef fmax
#undef fmaxl

static inline long double fpu_fmax(long double x, long double y){
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
	return (x > y) ? x : y;
}

float fmaxf(float x, float y){
#ifdef _WIN64
	return __MCFCRT_xmmmaxss(x, y);
#else
	return (float)fpu_fmax(x, y);
#endif
}
double fmax(double x, double y){
#ifdef _WIN64
	return __MCFCRT_xmmmaxsd(x, y);
#else
	return (double)fpu_fmax(x, y);
#endif
}
long double fmaxl(long double x, long double y){
	return fpu_fmax(x, y);
}
