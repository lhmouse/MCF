// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef fmaf
#undef fma
#undef fmal

// https://en.wikipedia.org/wiki/Extended_precision#x86_Extended_Precision_Format
typedef union tag_x87reg {
	struct __attribute__((__packed__)) {
		union {
			uint64_t f64;
			struct __attribute__((__packed__)) {
				uint32_t flo;
				uint32_t fhi;
			};
		};
		uint16_t exp : 15;
		uint16_t sgn :  1;
	};
	long double f;
} x87reg;

static inline void break_down(x87reg *restrict lo, x87reg *restrict hi, long double x){
	hi->f = x;
	const uint32_t flo = hi->flo;
	const long     exp = hi->exp;
	const bool     sgn = hi->sgn;
	hi->flo = 0;

	if(flo == 0){
		lo->f64 = 0;
		lo->exp = 0;
	} else {
		const long shn = _Generic(flo,
			unsigned:      __builtin_clz,
			unsigned long: __builtin_clzl)(flo) + 32;
		const long mask = (shn - exp) >> 31;
		long expm1 = exp - 1;
		expm1 &= ~(expm1 >> 31);
		lo->f64 = (uint64_t)flo << (((shn ^ expm1) & mask) ^ expm1);
		lo->exp = ((uint32_t)((exp - shn) & mask) << 17) >> 17;
	}
	lo->sgn = sgn;
}
static inline long double fpu_fma(long double x, long double y, long double z){
	x87reg xlo, xhi, ylo, yhi;
	break_down(&xlo, &xhi, x);
	break_down(&ylo, &yhi, y);
	long double ret = z;
	ret += xhi.f * yhi.f;
	ret += xhi.f * ylo.f + xlo.f * yhi.f;
	ret += xlo.f * ylo.f;
	return ret;
}

float fmaf(float x, float y, float z){
	return (float)fpu_fma(x, y, z);
}
double fma(double x, double y, double z){
	return (double)fpu_fma(x, y, z);
}
long double fmal(long double x, long double y, long double z){
	return fpu_fma(x, y, z);
}
