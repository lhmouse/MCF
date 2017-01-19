// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef copysignf
#undef copysign
#undef copysignl

static inline long double fpu_copysign(long double x, long double y){
	long double ret = __MCFCRT_fabs(x);
	bool sign;
	__MCFCRT_fxam(&sign, y);
	if(sign){
		ret = __MCFCRT_fchs(ret);
	}
	return ret;
}

float copysignf(float x, float y){
	float ret;
#ifdef _WIN64
	static const uint32_t mmask = (1ull << 31) - 1;
	__asm__ (
		"movss xmm2, dword ptr[%3] \n"
		"xorps xmm0, xmm1 \n"
		"andps xmm0, xmm2 \n"
		"xorps xmm0, xmm1 \n"
		: "=Yz"(ret)
		: "0"(x), "x"(y), "m"(mmask)
		: "xmm2"
	);
#else
	ret = (float)fpu_copysign(x, y);
#endif
	return ret;
}
double copysign(double x, double y){
	double ret;
#ifdef _WIN64
	static const uint64_t mmask = (1ull << 63) - 1;
	__asm__ (
		"movsd xmm2, qword ptr[%3] \n"
		"xorpd xmm0, xmm1 \n"
		"andpd xmm0, xmm2 \n"
		"xorpd xmm0, xmm1 \n"
		: "=Yz"(ret)
		: "0"(x), "x"(y), "m"(mmask)
		: "xmm2"
	);
#else
	ret = (double)fpu_copysign(x, y);
#endif
	return ret;
}
long double copysignl(long double x, long double y){
	long double ret;
	ret = fpu_copysign(x, y);
	return ret;
}
