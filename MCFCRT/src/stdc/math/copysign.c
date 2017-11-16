// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

#undef copysignf
#undef copysign
#undef copysignl

float copysignf(float x, float y){
	return __builtin_copysignf(x, y);
}
double copysign(double x, double y){
	return __builtin_copysign(x, y);
}
long double copysignl(long double x, long double y){
	return __builtin_copysignl(x, y);
}
