// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

extern float fdimf(float x, float y);
extern double fdim(double x, double y);
extern long double fdiml(long double x, long double y);

float fminf(float x, float y){
	return x - fdimf(x, y);
}

double fmin(double x, double y){
	return x - fdim(x, y);
}

long double fminl(long double x, long double y){
	return x - fdiml(x, y);
}
