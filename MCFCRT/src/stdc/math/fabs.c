// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

#undef fabsf
#undef fabs
#undef fabsl

float fabsf(float x){
	return __builtin_fabsf(x);
}
double fabs(double x){
	return __builtin_fabs(x);
}
long double fabsl(long double x){
	return __builtin_fabsl(x);
}
