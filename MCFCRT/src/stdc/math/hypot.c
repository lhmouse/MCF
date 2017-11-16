// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_asm_fpu.h"

#undef hypotf
#undef hypot
#undef hypotl

static inline long double fpu_hypot(long double x, long double y){
	bool xsign;
	const __MCFCRT_FpuExamine xexam = __MCFCRT_fxam(&xsign, x);
	long double xa = __MCFCRT_fabs(x);
	if(xexam == __MCFCRT_kFpuExamineInfinity){
		return xa;
	}
	bool ysign;
	const __MCFCRT_FpuExamine yexam = __MCFCRT_fxam(&ysign, y);
	long double ya = __MCFCRT_fabs(y);
	if(yexam == __MCFCRT_kFpuExamineInfinity){
		return ya;
	}
	if(xexam == __MCFCRT_kFpuExamineNaN){
		if(yexam == __MCFCRT_kFpuExamineNaN){
			// If both operands are NaNs, return the bitwise-and'd payload, which is a QNaN if and only if both operands are QNaNs.
			// That is because ISO C says `hypot(x, y)`, and `hypot(y, x)` are equivalent. We shall return the same NaN in these cases.
			// Despite inequality of a NaN with any value (including itself), it is possible to discriminate between the two cases above using `memcmp()`.
			__MCFCRT_X87Register xr, yr, rr;
			xr.__val = x;
			yr.__val = y;
			rr.__sign = (bool)(xr.__sign & yr.__sign);
			rr.__exp = 0x7FFF; // Special exponent for NaN.
			rr.__mts_q = xr.__mts_q & yr.__mts_q;
			return rr.__val;
		}
		return x;
	}
	if(yexam == __MCFCRT_kFpuExamineNaN){
		return y;
	}
	if(xexam == __MCFCRT_kFpuExamineZero){
		return ya;
	}
	if(yexam == __MCFCRT_kFpuExamineZero){
		return xa;
	}
	long double xn, yn, xm, ym;
	xm = __MCFCRT_fxtract(&xn, xa);
	ym = __MCFCRT_fxtract(&yn, ya);
	if(xn < yn){
		long double t;
		(t = xa), (xa = ya), (ya = t);
		(t = xn), (xn = yn), (yn = t);
		(t = xm), (xm = ym), (ym = t);
	}
	return __MCFCRT_fscale(__MCFCRT_fsqrt(__MCFCRT_fsquare(xm) + __MCFCRT_fsquare(__MCFCRT_fscale(ya, -xn))), xn);
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
