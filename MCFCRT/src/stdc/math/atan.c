// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef atanf
#undef atan
#undef atanl

float atanf(float x){
	return (float) __MCFCRT_fpatan(x, __MCFCRT_fld1());
}
double atan(double x){
	return (double)__MCFCRT_fpatan(x, __MCFCRT_fld1());
}
long double atanl(long double x){
	return __MCFCRT_fpatan(x, __MCFCRT_fld1());
}
