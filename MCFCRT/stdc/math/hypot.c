// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

extern float sqrtf(float x);
extern double sqrt(double x);
extern long double sqrtl(long double x);

float hypotf(float x, float y){
	return sqrtf(x * x + y * y);
}

double hypot(double x, double y){
	return sqrt(x * x + y * y);
}

long double hypotl(long double x, long double y){
	return sqrtl(x * x + y * y);
}
