// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

extern float scalblnf(float x, long n);
extern double scalbln(double x, long n);
extern long double scalblnl(long double x, long n);

float ldexpf(float x, int n){
	return scalblnf(x, n);
}

double ldexp(double x, int n){
	return scalbln(x, n);
}

long double ldexpl(long double x, int n){
	return scalblnl(x, n);
}
