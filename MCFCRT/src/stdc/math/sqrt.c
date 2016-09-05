// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

float sqrtf(float x){
	return (float) __MCFCRT_fsqrt(x);
}
double sqrt(double x){
	return (double)__MCFCRT_fsqrt(x);
}
long double sqrtl(long double x){
	return __MCFCRT_fsqrt(x);
}
