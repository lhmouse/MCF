// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

#undef signbitf
#undef signbit
#undef signbitl

int signbitf(float x){
	return __builtin_signbitf(x);
}
int signbit(double x){
	return __builtin_signbit(x);
}
int signbitl(long double x){
	return __builtin_signbitl(x);
}
