// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef ceilf
#undef ceil
#undef ceill

static inline long double fpu_ceil(long double x){
	bool sign;
	const __MCFCRT_FpuExamine exam = __MCFCRT_fxam(&sign, x);
	if(exam == __MCFCRT_kFpuExamineNaN){
		return x;
	}
	return __MCFCRT_fceil(x);
}

float ceilf(float x){
	return (float)fpu_ceil(x);
}
double ceil(double x){
	return (double)fpu_ceil(x);
}
long double ceill(long double x){
	return fpu_ceil(x);
}
