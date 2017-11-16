// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"
#include "_sse2.h"

#undef fabsf
#undef fabs
#undef fabsl

float fabsf(float x){
	float ret;
#ifdef _WIN64
	ret = __MCFCRT_xmmabsss(x);
#else
	ret = (float)__MCFCRT_fabs(x);
#endif
	return ret;
}
double fabs(double x){
	double ret;
#ifdef _WIN64
	ret = __MCFCRT_xmmabssd(x);
#else
	ret = (double)__MCFCRT_fabs(x);
#endif
	return ret;
}
long double fabsl(long double x){
	long double ret;
	ret = __MCFCRT_fabs(x);
	return ret;
}
