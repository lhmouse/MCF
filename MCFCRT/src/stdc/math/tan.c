// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef tanf
#undef tan
#undef tanl

static inline long double fpu_tan(long double x){
	bool invalid;
	long double ret = __MCFCRT_ftan(&invalid, x);
	if(invalid){
		ret = __MCFCRT_ftan(&invalid, __MCFCRT_trigonometric_reduce(ret));
	}
	return ret;
}

float tanf(float x){
	return (float)fpu_tan(x);
}
double tan(double x){
	return (double)fpu_tan(x);
}
long double tanl(long double x){
	return fpu_tan(x);
}
