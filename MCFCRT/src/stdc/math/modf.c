// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef modff
#undef modf
#undef modfl

float modff(float value, float *iptr){
	const long double x = value;
	const long double i = __MCFCRT_ftrunc(x);
	*iptr = (float)i;
	return (float)(x - i);
}
double modf(double value, double *iptr){
	const long double x = value;
	const long double i = __MCFCRT_ftrunc(x);
	*iptr = (double)i;
	return (double)(x - i);
}
long double modfl(long double value, long double *iptr){
	const long double x = value;
	const long double i = __MCFCRT_ftrunc(x);
	*iptr = i;
	return (x - i);
}
