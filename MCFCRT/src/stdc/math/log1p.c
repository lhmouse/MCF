// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

static inline long double fpu_log1p(long double x){
	// 1 - 2^0.5/2 = 0.29289321881345247559915563789515
	if((-0.2928932188l <= x) && (x <= 0.2928932188l)){
		return __MCFCRT_fyl2xp1(__MCFCRT_fldln2(), x);
	}
	return __MCFCRT_fyl2x(__MCFCRT_fldln2(), x + 1.0l);
}

float log1pf(float x){
	return (float)fpu_log1p(x);
}
double log1p(double x){
	return (double)fpu_log1p(x);
}
long double log1pl(long double x){
	return fpu_log1p(x);
}
