// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

static inline long double fpu_tan(long double x){
	long double ret;
	if(!__MCFCRT_ftan(&ret, x)){
		__MCFCRT_ftan(&ret, __MCFCRT_fmod(x, 0x1p61l * 3.1415926535897932384626433832795l));
	}
	return ret;
}

float tanf(float x){
	return (float)fpu_tan(x);
}
double tan(double x){
	return (double)fpu_tan(x);
}
long double tanl(long double x){
	return fpu_tan(x);
}
