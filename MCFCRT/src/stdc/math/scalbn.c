// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef scalbnf
#undef scalbn
#undef scalbnl

float scalbnf(float x, int n){
	return (float)__MCFCRT_fscale(x, n);
}
double scalbn(double x, int n){
	return (double)__MCFCRT_fscale(x, n);
}
long double scalbnl(long double x, int n){
	return __MCFCRT_fscale(x, n);
}
