// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef signbitf
#undef signbit
#undef signbitl

bool signbitf(float x){
	bool ret;
#ifdef _WIN64
	__asm__ (
		"movmskps eax, %1 \n"
		"and eax, 1 \n"
		: "=a"(ret)
		: "x"(x)
	);
#else
	__MCFCRT_fxam(&ret, x);
#endif
	return ret;
}
bool signbit(double x){
	bool ret;
#ifdef _WIN64
	__asm__ (
		"movmskpd eax, %1 \n"
		"and eax, 1 \n"
		: "=a"(ret)
		: "x"(x)
	);
#else
	__MCFCRT_fxam(&ret, x);
#endif
	return ret;
}
bool signbitl(long double x){
	bool ret;
	__MCFCRT_fxam(&ret, x);
	return ret;
}
