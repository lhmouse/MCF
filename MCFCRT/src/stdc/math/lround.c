// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef lroundf
#undef lround
#undef lroundl

static inline long fpu_lround(long double x){
	long ret = 0;
	if(x < 0){
		__MCFCRT_fisttpl(&ret, x - 0.5l);
	} else if(x > 0){
		__MCFCRT_fisttpl(&ret, x + 0.5l);
	}
	return ret;
}

long lroundf(float x){
	return fpu_lround(x);
}
long lround(double x){
	return fpu_lround(x);
}
long lroundl(long double x){
	return fpu_lround(x);
}
