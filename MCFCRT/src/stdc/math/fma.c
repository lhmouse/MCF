// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef fmaf
#undef fma
#undef fmal

static inline long double fpu_fma(long double x, long double y, long double z){
	return x * y + z;
}

float fmaf(float x, float y, float z){
	return (float)fpu_fma(x, y, z);
}
double fma(double x, double y, double z){
	return (double)fpu_fma(x, y, z);
}
long double fmal(long double x, long double y, long double z){
	return fpu_fma(x, y, z);
}
