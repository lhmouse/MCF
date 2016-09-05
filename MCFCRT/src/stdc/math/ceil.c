// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

float ceilf(float x){
	return (float)__MCFCRT_fceil(x);
}
double ceil(double x){
	return (double)__MCFCRT_fceil(x);
}
long double ceill(long double x){
	return __MCFCRT_fceil(x);
}
