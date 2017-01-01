// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef signbitf
#undef signbit
#undef signbitl

bool signbitf(float x){
#ifdef _WIN64
	bool ret;
	__asm__(
		"movmskps eax, %1 \n"
		"and eax, 1 \n"
		: "=a"(ret)
		: "x"(x)
	);
	return ret;
#else
	return __MCFCRT_fgetsign(x);
#endif
}
bool signbit(double x){
#ifdef _WIN64
	bool ret;
	__asm__(
		"movmskpd eax, %1 \n"
		"and eax, 1 \n"
		: "=a"(ret)
		: "x"(x)
	);
	return ret;
#else
	return __MCFCRT_fgetsign(x);
#endif
}
bool signbitl(long double x){
	return __MCFCRT_fgetsign(x);
}
