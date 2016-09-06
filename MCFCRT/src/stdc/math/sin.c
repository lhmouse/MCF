// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef sinf
#undef sin
#undef sinl

static inline long double fpu_sin(long double x){
	long double ret;
	if(!__MCFCRT_fsin(&ret, x)){
		int fsw;
		__MCFCRT_fsin(&ret, __MCFCRT_fmod(&fsw, x, 0x1p61l * 3.1415926535897932384626433832795l));
	}
	return ret;
}

float sinf(float x){
	return (float)fpu_sin(x);
}
double sin(double x){
	return (double)fpu_sin(x);
}
long double sinl(long double x){
	return fpu_sin(x);
}
