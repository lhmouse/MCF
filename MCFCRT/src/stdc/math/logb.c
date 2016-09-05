// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

float logbf(float x){
	long double exp;
	__MCFCRT_fxtract(&exp, x);
	return (float)exp;
}
double logb(double x){
	long double exp;
	__MCFCRT_fxtract(&exp, x);
	return (double)exp;
}
long double logbl(long double x){
	long double exp;
	__MCFCRT_fxtract(&exp, x);
	return exp;
}
