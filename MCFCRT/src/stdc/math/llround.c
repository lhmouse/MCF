// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_asm_fpu.h"
#include "_asm_sse3.h"

#undef llroundf
#undef llround
#undef llroundl

static inline long long fpu_llround(long double x){
	bool sign;
	__MCFCRT_fxam(&sign, x);
	long long ret;
	__MCFCRT_fisttpll(&ret, sign ? (x - 0.5l) : (x + 0.5l));
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
