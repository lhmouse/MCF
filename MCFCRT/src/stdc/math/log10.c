// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef log10f
#undef log10
#undef log10l

static inline long double fpu_log10(long double x){
	return __MCFCRT_fyl2x(__MCFCRT_fldlg2(), x);
}

float log10f(float x){
	return (float)fpu_log10(x);
}
double log10(double x){
	return (double)fpu_log10(x);
}
long double log10l(long double x){
	return fpu_log10(x);
}
