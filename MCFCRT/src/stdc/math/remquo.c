// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_asm_fpu.h"

#undef remquof
#undef remquo
#undef remquol

static inline long double fpu_remquo(long double x, long double y, int *quo){
	bool xsign;
	const __MCFCRT_FpuExamine xexam = __MCFCRT_fxam(&xsign, x);
	if(xexam == __MCFCRT_kFpuExamineNaN){
		return x;
	}
	if(xexam == __MCFCRT_kFpuExamineInfinity){
		return __builtin_nansl("0x4D43463A6672656D") + __MCFCRT_fldz();
	}
	bool ysign;
	const __MCFCRT_FpuExamine yexam = __MCFCRT_fxam(&ysign, y);
	if(yexam == __MCFCRT_kFpuExamineNaN){
		return y;
	}
	if(yexam == __MCFCRT_kFpuExamineZero){
		return __builtin_nansl("0x4D43463A6672656D") + __MCFCRT_fldz();
	}
	if(xexam == __MCFCRT_kFpuExamineZero){
		return x;
	}
	if(yexam == __MCFCRT_kFpuExamineInfinity){
		return x;
	}
	bool bits[3];
	const long double rem = __MCFCRT_fremainder(&bits, x, y);
	*quo = -((xsign ^ ysign) << 3) | (bits[2] << 2) | (bits[1] << 1) | bits[0];
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
