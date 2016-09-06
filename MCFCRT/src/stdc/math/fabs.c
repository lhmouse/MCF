// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

#undef fabsf
#undef fabs
#undef fabsl

static inline long double fpu_fabs(long double x){
	long double ret;
	__asm__(
		"fabs \n"
		: "=&t"(ret)
		: "0"(x)
	);
	return ret;
}

float fabsf(float x){
#ifdef _WIN64
	static alignas(16) const uint32_t mask = 0x7FFFFFFFu;
	float ret;
	__asm__(
		"movss xmm1, dword ptr[%2] \n"
		"andps xmm0, xmm1 \n"
		: "=Yz"(ret)
		: "0"(x), "m"(mask)
		: "xmm1"
	);
	return ret;
#else
	return (float)fpu_fabs(x);
#endif
}
double fabs(double x){
#ifdef _WIN64
	static alignas(16) const uint64_t mask = 0x7FFFFFFFFFFFFFFFu;
	double ret;
	__asm__(
		"movsd xmm1, qword ptr[%2] \n"
		"andpd xmm0, xmm1 \n"
		: "=Yz"(ret)
		: "0"(x), "m"(mask)
		: "xmm1"
	);
	return ret;
#else
	return (double)fpu_fabs(x);
#endif
}
long double fabsl(long double x){
	return fpu_fabs(x);
}
