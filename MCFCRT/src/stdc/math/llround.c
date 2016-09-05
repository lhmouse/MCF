// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

static inline long long real_llround(long double x){
	if(x < 0){
		return (long long)__MCFCRT_ftrunc(x - 0.5l);
	} else if(x > 0){
		return (long long)__MCFCRT_ftrunc(x + 0.5l);
	} else {
		return 0;
	}
}

long long llroundf(float x){
	return real_llround(x);
}
long long llround(double x){
	return real_llround(x);
}
long long llroundl(long double x){
	return real_llround(x);
}
