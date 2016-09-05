// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

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
	int fsw;
	const long double rem = __MCFCRT_fremainder(&fsw, x, y);
#if __FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__
	const int sign = (int8_t)((((const int8_t *)&x)[9] ^ ((const int8_t *)&y)[9]) & 0x80);
#else
	const int sign = (int8_t)((((const int8_t *)&x)[0] ^ ((const int8_t *)&y)[0]) & 0x80);
#endif
	const int bits = quo_table[(fsw >> 8) & 0xFF];
	*quo = sign + bits;
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
