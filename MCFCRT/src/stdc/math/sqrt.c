// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_asm_fpu.h"
#include "_asm_sse2.h"

#undef sqrtf
#undef sqrt
#undef sqrtl

float sqrtf(float x){
#ifdef _WIN64
	return __MCFCRT_xmmsqrtss(x);
#else
	return (float)__MCFCRT_fsqrt(x);
#endif
}
double sqrt(double x){
#ifdef _WIN64
	return __MCFCRT_xmmsqrtsd(x);
#else
	return (double)__MCFCRT_fsqrt(x);
#endif
}
long double sqrtl(long double x){
	return __MCFCRT_fsqrt(x);
}
