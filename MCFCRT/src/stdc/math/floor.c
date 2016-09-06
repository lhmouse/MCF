// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef floorf
#undef floor
#undef floorl

float floorf(float x){
	return (float)__MCFCRT_ffloor(x);
}
double floor(double x){
	return (double)__MCFCRT_ffloor(x);
}
long double floorl(long double x){
	return __MCFCRT_ffloor(x);
}
