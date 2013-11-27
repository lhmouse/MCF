// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

__MCF_CRT_EXTERN float truncf(float x);
__MCF_CRT_EXTERN double trunc(double x);
__MCF_CRT_EXTERN long double truncl(long double x);

__MCF_CRT_EXTERN float modff(float value, float *iptr){
	return value - (*iptr = truncf(value));
}

__MCF_CRT_EXTERN double modf(double value, double *iptr){
	return value - (*iptr = trunc(value));
}

__MCF_CRT_EXTERN long double modfl(long double value, long double *iptr){
	return value - (*iptr = truncl(value));
}
