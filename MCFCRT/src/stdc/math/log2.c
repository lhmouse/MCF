// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef log2f
#undef log2
#undef log2l

static inline long double fpu_log2(long double x){
	return __MCFCRT_fyl2x(1.0l, x);
}

float log2f(float x){
	return (float)fpu_log2(x);
}
double log2(double x){
	return (double)fpu_log2(x);
}
long double log2l(long double x){
	return fpu_log2(x);
}
