// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef remquof
#undef remquo
#undef remquol

static const uint8_t quo_table[256] = {
	0, 4, 1, 5, 0, 4, 1, 5, 0, 4, 1, 5, 0, 4, 1, 5,
	0, 4, 1, 5, 0, 4, 1, 5, 0, 4, 1, 5, 0, 4, 1, 5,
	0, 4, 1, 5, 0, 4, 1, 5, 0, 4, 1, 5, 0, 4, 1, 5,
	0, 4, 1, 5, 0, 4, 1, 5, 0, 4, 1, 5, 0, 4, 1, 5,
	2, 6, 3, 7, 2, 6, 3, 7, 2, 6, 3, 7, 2, 6, 3, 7,
	2, 6, 3, 7, 2, 6, 3, 7, 2, 6, 3, 7, 2, 6, 3, 7,
	2, 6, 3, 7, 2, 6, 3, 7, 2, 6, 3, 7, 2, 6, 3, 7,
	2, 6, 3, 7, 2, 6, 3, 7, 2, 6, 3, 7, 2, 6, 3, 7,
	0, 4, 1, 5, 0, 4, 1, 5, 0, 4, 1, 5, 0, 4, 1, 5,
	0, 4, 1, 5, 0, 4, 1, 5, 0, 4, 1, 5, 0, 4, 1, 5,
	0, 4, 1, 5, 0, 4, 1, 5, 0, 4, 1, 5, 0, 4, 1, 5,
	0, 4, 1, 5, 0, 4, 1, 5, 0, 4, 1, 5, 0, 4, 1, 5,
	2, 6, 3, 7, 2, 6, 3, 7, 2, 6, 3, 7, 2, 6, 3, 7,
	2, 6, 3, 7, 2, 6, 3, 7, 2, 6, 3, 7, 2, 6, 3, 7,
	2, 6, 3, 7, 2, 6, 3, 7, 2, 6, 3, 7, 2, 6, 3, 7,
	2, 6, 3, 7, 2, 6, 3, 7, 2, 6, 3, 7, 2, 6, 3, 7,
};

static inline long double fpu_remquo(long double x, long double y, int *quo){
	const bool neg = ((__MCFCRT_ftest(x) == __MCFCRT_kFpuNegative) ^ (__MCFCRT_ftest(y) == __MCFCRT_kFpuNegative));
	unsigned fsw;
	const long double rem = __MCFCRT_fremainder(&fsw, x, y);
	const int sign = -(neg << 3);
	const int bits = quo_table[(fsw >> 8) & 0xFF];
	*quo = sign | bits;
	return rem;
}

float remquof(float x, float y, int *quo){
	return (float)fpu_remquo(x, y, quo);
}
double remquo(double x, double y, int *quo){
	return (double)fpu_remquo(x, y, quo);
}
long double remquol(long double x, long double y, int *quo){
	return fpu_remquo(x, y, quo);
}
