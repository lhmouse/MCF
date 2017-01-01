// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef atan2f
#undef atan2
#undef atan2l

float atan2f(float y, float x){
	return (float) __MCFCRT_fpatan(y, x);
}
double atan2(double y, double x){
	return (double)__MCFCRT_fpatan(y, x);
}
long double atan2l(long double y, long double x){
	return __MCFCRT_fpatan(y, x);
}
