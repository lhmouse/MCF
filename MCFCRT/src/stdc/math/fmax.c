// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

extern float fdimf(float x, float y);
extern double fdim(double x, double y);
extern long double fdiml(long double x, long double y);

float fmaxf(float x, float y){
	return x + fdimf(y, x);
}

double fmax(double x, double y){
	return x + fdim(y, x);
}

long double fmaxl(long double x, long double y){
	return x + fdiml(y, x);
}
