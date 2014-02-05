// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

extern float truncf(float x);
extern double trunc(double x);
extern long double truncl(long double x);

float modff(float value, float *iptr){
	return value - (*iptr = truncf(value));
}

double modf(double value, double *iptr){
	return value - (*iptr = trunc(value));
}

long double modfl(long double value, long double *iptr){
	return value - (*iptr = truncl(value));
}
