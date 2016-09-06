// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef logbf
#undef logb
#undef logbl

float logbf(float x){
	long double n;
	__MCFCRT_fxtract(&n, x);
	return (float)n;
}
double logb(double x){
	long double n;
	__MCFCRT_fxtract(&n, x);
	return (double)n;
}
long double logbl(long double x){
	long double n;
	__MCFCRT_fxtract(&n, x);
	return n;
}
