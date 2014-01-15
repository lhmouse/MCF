// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

__MCF_CRT_EXTERN float scalblnf(float x, long n);
__MCF_CRT_EXTERN double scalbln(double x, long n);
__MCF_CRT_EXTERN long double scalblnl(long double x, long n);

__MCF_CRT_EXTERN float ldexpf(float x, int n){
	return scalblnf(x, n);
}

__MCF_CRT_EXTERN double ldexp(double x, int n){
	return scalbln(x, n);
}

__MCF_CRT_EXTERN long double ldexpl(long double x, int n){
	return scalblnl(x, n);
}
