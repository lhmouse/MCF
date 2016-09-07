// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef tanf
#undef tan
#undef tanl

static inline long double fpu_tan(long double x){
	unsigned fsw;
	const long double reduced = __MCFCRT_fremainder(&fsw, x, __MCFCRT_fldpi());
	long double ret = __MCFCRT_ftan_unsafe(reduced);
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
