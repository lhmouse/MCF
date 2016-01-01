// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include <limits.h>
#include <math.h>

extern float logbf(float x);
extern double logb(double x);
extern long double logbl(long double x);

int ilogbf(float x){
	if(x == 0){
		return FP_ILOGB0;
	}
	if(__builtin_isnanf(x)){
		return FP_ILOGBNAN;
	}

	if(__builtin_isinff(x)){
		return INT_MAX;
	}
	return (int)logbf(x);
}

int ilogb(double x){
	if(x == 0){
		return FP_ILOGB0;
	}
	if(__builtin_isnan(x)){
		return FP_ILOGBNAN;
	}

	if(__builtin_isinf(x)){
		return INT_MAX;
	}
	return (int)logb(x);
}

int ilogbl(long double x){
	if(x == 0){
		return FP_ILOGB0;
	}
	if(__builtin_isnanl(x)){
		return FP_ILOGBNAN;
	}

	if(__builtin_isinfl(x)){
		return INT_MAX;
	}
	return (int)logbl(x);
}
