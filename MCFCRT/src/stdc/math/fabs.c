// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef fabsf
#undef fabs
#undef fabsl

float fabsf(float x){
#ifdef _WIN64
	static const uint32_t mmask = 0x7FFFFFFFu;
	float ret;
	__asm__(
		"movss xmm1, dword ptr[%2] \n"
		"andps xmm0, xmm1 \n"
		: "=Yz"(ret)
		: "0"(x), "m"(mmask)
		: "xmm1"
	);
	return ret;
#else
	return (float)__MCFCRT_fabs(x);
#endif
}
double fabs(double x){
#ifdef _WIN64
	static const uint64_t mmask = 0x7FFFFFFFFFFFFFFFu;
	double ret;
	__asm__(
		"movsd xmm1, qword ptr[%2] \n"
		"andpd xmm0, xmm1 \n"
		: "=Yz"(ret)
		: "0"(x), "m"(mmask)
		: "xmm1"
	);
	return ret;
#else
	return (double)__MCFCRT_fabs(x);
#endif
}
long double fabsl(long double x){
	return __MCFCRT_fabs(x);
}
