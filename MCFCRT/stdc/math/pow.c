// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../env/bail.h"
#include "_math_asm.h"
#include <limits.h>

#define UNROLLED	\
		if(++i == 32){	\
			break;	\
		}	\
		ret *= ret;	\
		mask >>= 1;	\
		if((y & mask) != 0){	\
			ret *= x;	\
		}

// postive pow float unsigned
static float ppowfu(float x, unsigned y){
	if(y == 0){
		return 1.0f;
	}
	register float ret = x;
	unsigned i = (unsigned)__builtin_clz(y);
	unsigned mask = 0x80000000u >> i;
	for(;;){
		UNROLLED
		UNROLLED
		UNROLLED
		UNROLLED
		UNROLLED
		UNROLLED
		UNROLLED
		UNROLLED
	}
	return ret;
}
static double ppowdu(double x, unsigned y){
	if(y == 0){
		return 1.0;
	}
	register double ret = x;
	unsigned i = (unsigned)__builtin_clz(y);
	unsigned mask = 0x80000000u >> i;
	for(;;){
		UNROLLED
		UNROLLED
		UNROLLED
		UNROLLED
		UNROLLED
		UNROLLED
		UNROLLED
		UNROLLED
	}
	return ret;
}
static long double ppowlu(long double x, unsigned y){
	if(y == 0){
		return 1.0l;
	}
	register long double ret = x;
	unsigned i = (unsigned)__builtin_clz(y);
	unsigned mask = 0x80000000u >> i;
	for(;;){
		UNROLLED
		UNROLLED
		UNROLLED
		UNROLLED
		UNROLLED
		UNROLLED
		UNROLLED
		UNROLLED
	}
	return ret;
}

#define PPOWU(t_, x_, y_)	\
	_Generic((t_)0,	\
		float:   ppowfu,	\
		double:  ppowdu,	\
		default: ppowlu)(x_, y_)

// postive pow float float
static float ppowf(float x, float y){
	register float ret;
	__asm__ __volatile__(
		"fld dword ptr[%2] \n"
		"fld dword ptr[%1] \n"
		"fyl2x \n"
		"fld st \n"
		"frndint \n"
		"fsub st(1), st \n"
		"fld1 \n"
		"fscale \n"
		"fstp st(1) \n"
		"fxch st(1) \n"
		"f2xm1 \n"
		"fld1 \n"
		"faddp st(1), st \n"
		"fmulp st(1), st \n"
		__FLT_RET_ST("%1")
		: __FLT_RET_CONS(ret)
		: "m"(x), "m"(y)
	);
	return ret;
}
static double ppowd(double x, double y){
	register double ret;
	__asm__ __volatile__(
		"fld qword ptr[%2] \n"
		"fld qword ptr[%1] \n"
		"fyl2x \n"
		"fld st \n"
		"frndint \n"
		"fsub st(1), st \n"
		"fld1 \n"
		"fscale \n"
		"fstp st(1) \n"
		"fxch st(1) \n"
		"f2xm1 \n"
		"fld1 \n"
		"faddp st(1), st \n"
		"fmulp st(1), st \n"
		__DBL_RET_ST("%1")
		: __DBL_RET_CONS(ret)
		: "m"(x), "m"(y)
	);
	return ret;
}
static long double ppowl(double x, double y){
	register long double ret;
	__asm__ __volatile__(
		"fld tbyte ptr[%2] \n"
		"fld tbyte ptr[%1] \n"
		"fyl2x \n"
		"fld st \n"
		"frndint \n"
		"fsub st(1), st \n"
		"fld1 \n"
		"fscale \n"
		"fstp st(1) \n"
		"fxch st(1) \n"
		"f2xm1 \n"
		"fld1 \n"
		"faddp st(1), st \n"
		"fmulp st(1), st \n"
		__LDBL_RET_ST("%1")
		: __LDBL_RET_CONS(ret)
		: "m"(x), "m"(y)
	);
	return ret;
}

#define PPOW(t_, x_, y_)	\
	_Generic((t_)0,	\
		float:   ppowf,	\
		double:  ppowd,	\
		default: ppowl)(x_, y_)

#define POW_IMPL(t_)	\
	if(y == 0){	\
		return 1.0;	\
	}	\
	if(x == 0){	\
		return 0.0;	\
	}	\
	const t_ whole = _Generic((t_)0,	\
		float:   __builtin_floorf,	\
		double:  __builtin_floor,	\
		default: __builtin_floorl)(y);	\
	const t_ frac = y - whole;	\
	if(x > 0){	\
		if(y > (t_)INT_MAX){	\
			return PPOW(t_, x, y);	\
		} else if(y < (t_)INT_MIN){	\
			return 1.0 / PPOW(t_, x, -y);	\
		} else {	\
			t_ ret;	\
			if(y > 0){	\
				ret = PPOWU(t_, x, (unsigned)whole);	\
			} else {	\
				ret = 1.0 / PPOWU(t_, x, (unsigned)-whole);	\
			}	\
			if(frac != 0){	\
				ret *= PPOW(t_, x, frac);	\
			}	\
			return ret;	\
		}	\
	} else {	\
		if(frac != 0){	\
			MCF_CRT_Bail(L"只能求负数的整数次幂。");	\
		}	\
		if(y > (t_)INT_MAX){	\
			if(_Generic((t_)0,	\
				float:   __builtin_fmodf,	\
				double:  __builtin_fmod,	\
				default: __builtin_fmodl)(whole, 2.0) == 0.0)	\
			{	\
				return PPOW(t_, -x, whole);	\
			} else {	\
				return -PPOW(t_, -x, whole);	\
			}	\
		} else if(y < (t_)INT_MIN){	\
			if(_Generic((t_)0,	\
				float:   __builtin_fmodf,	\
				double:  __builtin_fmod,	\
				default: __builtin_fmodl)(whole, 2.0) == 0.0)	\
			{	\
				return 1.0 / PPOW(t_, -x, whole);	\
			} else {	\
				return -1.0 / PPOW(t_, -x, whole);	\
			}	\
		} else {	\
			if(whole > 0){	\
				const unsigned idx = (unsigned)whole;	\
				if(idx % 2 == 0){	\
					return PPOWU(t_, -x, idx);	\
				} else {	\
					return -PPOWU(t_, -x, idx);	\
				}	\
			} else {	\
				const unsigned idx = (unsigned)-whole;	\
				if(idx % 2 == 0){	\
					return 1.0 / PPOWU(t_, -x, idx);	\
				} else {	\
					return -1.0 / PPOWU(t_, -x, idx);	\
				}	\
			}	\
		}	\
	}

float powf(float x, float y){
	POW_IMPL(float)
}
double pow(double x, double y){
	POW_IMPL(double)
}
long double powl(long double x, long double y){
	POW_IMPL(long double)
}
