// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_asm_fpu.h"
#include "_asm_sse2.h"

#undef copysignf
#undef copysign
#undef copysignl

static inline long double fpu_copysign(long double x, long double y){
	long double ret = __MCFCRT_fabs(x);
	bool sign;
	__MCFCRT_fxam(&sign, y);
	if(sign){
		ret = __MCFCRT_fchs(ret);
	}
	return ret;
}

float copysignf(float x, float y){
	float ret;
#ifdef _WIN64
	ret = __MCFCRT_xmmcopysignss(x, y);
#else
	ret = (float)fpu_copysign(x, y);
#endif
	return ret;
}
double copysign(double x, double y){
	double ret;
#ifdef _WIN64
	ret = __MCFCRT_xmmcopysignsd(x, y);
#else
	ret = (double)fpu_copysign(x, y);
#endif
	return ret;
}
long double copysignl(long double x, long double y){
	long double ret;
	ret = fpu_copysign(x, y);
	return ret;
}
