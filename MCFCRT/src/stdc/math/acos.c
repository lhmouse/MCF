// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef acosf
#undef acos
#undef acosl

static inline long double fpu_acos(long double x){
	return __MCFCRT_fpatan(__MCFCRT_fsqrt(1.0l - x * x), x);
}

float acosf(float x){
	return (float)fpu_acos(x);
}
double acos(double x){
	return (double)fpu_acos(x);
}
long double acosl(long double x){
	return fpu_acos(x);
}
