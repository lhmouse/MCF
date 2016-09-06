// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef copysignf
#undef copysign
#undef copysignl

static inline long double fpu_copysign(long double x, long double y){
	long double ret = __MCFCRT_fabs(x);
	if(__MCFCRT_fgetsign(y)){
		ret = __MCFCRT_fneg(ret);
	}
	return ret;
}

float copysignf(float x, float y){
	return (float)fpu_copysign(x, y);
}
double copysign(double x, double y){
	return (double)fpu_copysign(x, y);
}
long double copysignl(long double x, long double y){
	return (long double)fpu_copysign(x, y);
}
