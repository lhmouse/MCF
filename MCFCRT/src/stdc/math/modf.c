// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef modff
#undef modf
#undef modfl

static inline long double fpu_modf(long double value, long double *iptr){
	bool sign;
	const __MCFCRT_FpuExamine exam = __MCFCRT_fxam(&sign, value);
	if(exam == __MCFCRT_kFpuExamineNaN){
		*iptr = value;
		return value;
	}
	if(exam == __MCFCRT_kFpuExamineInfinity){
		if(sign){
			*iptr = value;
			return __MCFCRT_fchs(__MCFCRT_fldz());
		}
		*iptr = value;
		return __MCFCRT_fldz();
	}
	const long double i = __MCFCRT_ftrunc(value);
	*iptr = i;
	return value - i;
}

float modff(float value, float *iptr){
	long double i;
	const long double ret = fpu_modf(value, &i);
	*iptr = (float)i;
	return (float)ret;
}
double modf(double value, double *iptr){
	long double i;
	const long double ret = fpu_modf(value, &i);
	*iptr = (double)i;
	return (double)ret;
}
long double modfl(long double value, long double *iptr){
	return fpu_modf(value, iptr);
}
