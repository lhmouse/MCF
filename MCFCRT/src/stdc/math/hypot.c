// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef hypotf
#undef hypot
#undef hypotl

static inline long double fpu_hypot(long double x, long double y){
	bool xsign;
	const __MCFCRT_FpuExamine xexam = __MCFCRT_fxam(&xsign, x);
	if(xexam == __MCFCRT_kFpuExamineInfinity){
		return __MCFCRT_fabs(x);
	}
	bool ysign;
	const __MCFCRT_FpuExamine yexam = __MCFCRT_fxam(&ysign, y);
	if(yexam == __MCFCRT_kFpuExamineInfinity){
		return __MCFCRT_fabs(y);
	}
	if(xexam == __MCFCRT_kFpuExamineNaN){
		return x;
	}
	if(yexam == __MCFCRT_kFpuExamineNaN){
		return y;
	}
	long double xn, xm, yn, ym;
	xm = __MCFCRT_fxtract(&xn, x);
	ym = __MCFCRT_fxtract(&yn, y);
	if(xn > yn){
		return __MCFCRT_fscale(__MCFCRT_fsqrt(__MCFCRT_fsquare(xm) +
		                                      __MCFCRT_fsquare(__MCFCRT_fscale(y, -xn))),
		                       xn);
	} else {
		return __MCFCRT_fscale(__MCFCRT_fsqrt(__MCFCRT_fsquare(__MCFCRT_fscale(x, -yn)) +
		                                      __MCFCRT_fsquare(ym)),
		                       yn);
	}
}

float hypotf(float x, float y){
	return (float)fpu_hypot(x, y);
}
double hypot(double x, double y){
	return (double)fpu_hypot(x, y);
}
long double hypotl(long double x, long double y){
	return fpu_hypot(x, y);
}
