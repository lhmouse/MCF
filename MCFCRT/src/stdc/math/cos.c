// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef cosf
#undef cos
#undef cosl

static inline long double fpu_cos(long double x){
	bool invalid;
	long double ret = __MCFCRT_fcos(&invalid, x);
	if(invalid){
		ret = __MCFCRT_fcos(&invalid, __MCFCRT_trigonometric_reduce(ret));
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
