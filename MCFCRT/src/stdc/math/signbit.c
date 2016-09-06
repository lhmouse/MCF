// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef signbitf
#undef signbit
#undef signbitl

bool signbitf(float x){
	return __MCFCRT_fgetsign(x);
}
bool signbit(double x){
	return __MCFCRT_fgetsign(x);
}
bool signbitl(long double x){
	return __MCFCRT_fgetsign(x);
}
