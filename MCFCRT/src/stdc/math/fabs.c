// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef fabsf
#undef fabs
#undef fabsl

float fabsf(float x){
	float ret;
#ifdef _WIN64
	static const uint32_t mmask = (1ull << 31) - 1;
	__asm__ (
		"movss xmm1, dword ptr[%2] \n"
		"andps xmm0, xmm1 \n"
		: "=Yz"(ret)
		: "0"(x), "m"(mmask)
		: "xmm1"
	);
#else
	ret = (float)__MCFCRT_fabs(x);
#endif
	return ret;
}
double fabs(double x){
	double ret;
#ifdef _WIN64
	static const uint64_t mmask = (1ull << 63) - 1;
	__asm__ (
		"movsd xmm1, qword ptr[%2] \n"
		"andpd xmm0, xmm1 \n"
		: "=Yz"(ret)
		: "0"(x), "m"(mmask)
		: "xmm1"
	);
#else
	ret = (double)__MCFCRT_fabs(x);
#endif
	return ret;
}
long double fabsl(long double x){
	long double ret;
	ret = __MCFCRT_fabs(x);
	return ret;
}
