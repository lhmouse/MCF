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
	long double ax = __MCFCRT_fabs(x);
	if(xexam == __MCFCRT_kFpuExamineInfinity){
		return ax;
	}
	bool ysign;
	const __MCFCRT_FpuExamine yexam = __MCFCRT_fxam(&ysign, y);
	long double ay = __MCFCRT_fabs(y);
	if(yexam == __MCFCRT_kFpuExamineInfinity){
		return ay;
	}
	if(xexam == __MCFCRT_kFpuExamineNaN){
		if(yexam == __MCFCRT_kFpuExamineNaN){
			// If both operands are NaNs, return the bitwise-and'd payload. The result is a QNaN if and only if both operands are QNaNs.
			// That is because ISO C says `hypot(x, y)`, `hypot(y, x)`, and `hypot(x, -y)` are equivalent.
			// We should return the same NaN in this case. Note that a NaN compares unequal with any value, including itself.
			// This difference can be distinguished using `memcmp()`.
			__MCFCRT_x87Register xr, yr, rr;
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
		return ay;
	}
	if(yexam == __MCFCRT_kFpuExamineZero){
		return ax;
	}
	long double nx, ny, mx, my;
	mx = __MCFCRT_fxtract(&nx, ax);
	my = __MCFCRT_fxtract(&ny, ay);
	if(nx < ny){
		long double t;
		(t = ax), (ax = ay), (ay = t);
		(t = nx), (nx = ny), (ny = t);
		(t = mx), (mx = my), (my = t);
	}
	return __MCFCRT_fscale(__MCFCRT_fsqrt(__MCFCRT_fsquare(mx) + __MCFCRT_fsquare(__MCFCRT_fscale(ay, -nx))), nx);
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
