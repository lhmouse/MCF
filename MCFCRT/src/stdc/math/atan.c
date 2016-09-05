// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

float atanf(float x){
	return (float) __MCFCRT_fpatan(x, 1.0l);
}
double atan(double x){
	return (double)__MCFCRT_fpatan(x, 1.0l);
}
long double atanl(long double x){
	return __MCFCRT_fpatan(x, 1.0l);
}
