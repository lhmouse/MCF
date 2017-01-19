// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef fdimf
#undef fdim
#undef fdiml

static inline long double fpu_fdim(long double x, long double y){
	return (x > y) ? (x - y) : __MCFCRT_fldz();
}

float fdimf(float x, float y){
	return (float)fpu_fdim(x, y);
}
double fdim(double x, double y){
	return (double)fpu_fdim(x, y);
}
long double fdiml(long double x, long double y){
	return fpu_fdim(x, y);
}
