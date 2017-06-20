// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef fmaf
#undef fma
#undef fmal

static inline long saturated_dec(long val){
	long ret = val - 1;
	ret &= ~(ret >> 31);
	return ret;
}
static inline void break_down(__MCFCRT_x87Register *restrict lo, __MCFCRT_x87Register *restrict hi, long double x){
	hi->__val = x;
	const uint32_t mts_l = hi->__mts_l;
	const long exp = hi->__exp;
	const bool sign = hi->__sign;
	hi->__mts_l = 0;

	if(mts_l == 0){
		lo->__mts_q = 0;
		lo->__exp = 0;
	} else {
		const long shn = _Generic(mts_l,
			unsigned:      __builtin_clz,
			unsigned long: __builtin_clzl)(mts_l) + 32;
		const long mask = (shn - exp) >> 31;
		const long expm1_sat = saturated_dec(exp);
		lo->__mts_q = (uint64_t)mts_l << (((shn ^ expm1_sat) & mask) ^ expm1_sat);
		lo->__exp = ((uint32_t)((exp - shn) & mask) << 17) >> 17;
	}
	lo->__sign = sign;
}
static inline long double fpu_fma(long double x, long double y, long double z){
	bool xsign;
	const __MCFCRT_FpuExamine xexam = __MCFCRT_fxam(&xsign, x);
	if(xexam == __MCFCRT_kFpuExamineNaN){
		return x;
	}
	bool ysign;
	const __MCFCRT_FpuExamine yexam = __MCFCRT_fxam(&ysign, y);
	if(yexam == __MCFCRT_kFpuExamineNaN){
		return y;
	}
	bool zsign;
	const __MCFCRT_FpuExamine zexam = __MCFCRT_fxam(&zsign, z);
	if(zexam == __MCFCRT_kFpuExamineNaN){
		return z;
	}
	long double ret = x * y + z;
	bool rsign;
	const __MCFCRT_FpuExamine rexam = __MCFCRT_fxam(&rsign, ret);
	if(rexam == __MCFCRT_kFpuExamineNaN){
		return ret;
	}
	if(rexam == __MCFCRT_kFpuExamineInfinity){
		return ret;
	}
	__MCFCRT_x87Register xlo, xhi, ylo, yhi;
	break_down(&xlo, &xhi, x);
	break_down(&ylo, &yhi, y);
	ret = z;
	ret += xhi.__val * yhi.__val;
	ret += xhi.__val * ylo.__val + xlo.__val * yhi.__val;
	ret += xlo.__val * ylo.__val;
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
