// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

static inline long real_lround(long double x){
	if(x < 0){
		return (long)__MCFCRT_ftrunc(x - 0.5l);
	} else if(x > 0){
		return (long)__MCFCRT_ftrunc(x + 0.5l);
	} else {
		return 0;
	}
}

long lroundf(float x){
	return real_lround(x);
}
long lround(double x){
	return real_lround(x);
}
long lroundl(long double x){
	return real_lround(x);
}
