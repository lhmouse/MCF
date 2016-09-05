// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

static inline long double fpu_cos(long double x){
	long double ret;
	if(!__MCFCRT_fcos(&ret, x)){
		__MCFCRT_fcos(&ret, __MCFCRT_fmod(x, 0x1p61l * 3.1415926535897932384626433832795l));
	}
	return ret;
}

float cosf(float x){
	return (float)fpu_cos(x);
}
double cos(double x){
	return (double)fpu_cos(x);
}
long double cosl(long double x){
	return fpu_cos(x);
}
