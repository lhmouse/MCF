// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef fmodf
#undef fmod
#undef fmodl

static inline long double fpu_fmod(long double x, long double y){
	bool xsign;
	const __MCFCRT_FpuExamine xexam = __MCFCRT_fxam(&xsign, x);
	if(xexam == __MCFCRT_kFpuExamineNaN){
		return x;
	}
	if(xexam == __MCFCRT_kFpuExamineInfinity){
		return __builtin_nansl("0x4D43463A666D6F64") + __MCFCRT_fldz();
	}
	bool ysign;
	const __MCFCRT_FpuExamine yexam = __MCFCRT_fxam(&ysign, y);
	if(yexam == __MCFCRT_kFpuExamineNaN){
		return y;
	}
	if(yexam == __MCFCRT_kFpuExamineZero){
		return __builtin_nansl("0x4D43463A666D6F64") + __MCFCRT_fldz();
	}
	if(xexam == __MCFCRT_kFpuExamineZero){
		return x;
	}
	if(yexam == __MCFCRT_kFpuExamineInfinity){
		return x;
	}
	bool bits[3];
	return __MCFCRT_fmod(&bits, x, y);
}

float fmodf(float x, float y){
	return (float)fpu_fmod(x, y);
}
double fmod(double x, double y){
	return (double)fpu_fmod(x, y);
}
long double fmodl(long double x, long double y){
	return fpu_fmod(x, y);
}
