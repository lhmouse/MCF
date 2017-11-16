// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_asm_fpu.h"
#include "_asm_sse3.h"

#undef llroundf
#undef llround
#undef llroundl

static inline long fpu_lround(long double x){
	bool sign;
	__MCFCRT_fxam(&sign, x);
	long ret;
	__MCFCRT_fisttpl(&ret, sign ? (x - 0.5l) : (x + 0.5l));
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
