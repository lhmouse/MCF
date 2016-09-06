// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef ldexpf
#undef ldexp
#undef ldexpl

float ldexpf(float x, int n){
	return (float)__MCFCRT_fscale(x, n);
}
double ldexp(double x, int n){
	return (double)__MCFCRT_fscale(x, n);
}
long double ldexpl(long double x, int n){
	return __MCFCRT_fscale(x, n);
}
