// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"
#include "../../ext/expect.h"

#undef hypotf
#undef hypot
#undef hypotl

static inline long double fpu_hypot(long double x, long double y){
	long double cx = __MCFCRT_fabs(x), cy = __MCFCRT_fabs(y);
	if(cx < cy){
		long double temp = cx;
		cx = cy;
		cy = temp;
	}
	int scale = 0;
	if(cx >= 0x1p+8191l){
		scale = -8192;
	} else if((cx < __MCFCRT_fld1()) && (cy <= 0x1p-8191l)){
		scale = 8192;
	}
	if(_MCFCRT_EXPECT(scale == 0)){
		return __MCFCRT_fsqrt(__MCFCRT_fsquare(cx) +
		                      __MCFCRT_fsquare(cy));
	} else {
		return __MCFCRT_fscale(__MCFCRT_fsqrt(__MCFCRT_fsquare(__MCFCRT_fscale(cx, scale)) +
		                                      __MCFCRT_fsquare(__MCFCRT_fscale(cy, scale))),
		                       -scale);
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
