// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef truncf
#undef trunc
#undef truncl

float truncf(float x){
	return (float)__MCFCRT_ftrunc(x);
}
double trunc(double x){
	return (double)__MCFCRT_ftrunc(x);
}
long double truncl(long double x){
	return __MCFCRT_ftrunc(x);
}
