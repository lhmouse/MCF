// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef llroundf
#undef llround
#undef llroundl

static inline long long fpu_llround(long double x){
	long long ret = 0;
	if(x < 0){
		__MCFCRT_fistpll(&ret, __MCFCRT_ftrunc(x - 0.5l));
	} else if(x > 0){
		__MCFCRT_fistpll(&ret, __MCFCRT_ftrunc(x + 0.5l));
	}
	return ret;
}

long long llroundf(float x){
	return fpu_llround(x);
}
long long llround(double x){
	return fpu_llround(x);
}
long long llroundl(long double x){
	return fpu_llround(x);
}
