// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

static inline long double real_hypot(long double x, long double y){
	return __MCFCRT_fsqrt(x * x + y * y);
}

float hypotf(float x, float y){
	return (float)real_hypot(x, y);
}
double hypot(double x, double y){
	return (double)real_hypot(x, y);
}
long double hypotl(long double x, long double y){
	return real_hypot(x, y);
}
