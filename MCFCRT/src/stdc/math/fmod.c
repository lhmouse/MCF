// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef fmodf
#undef fmod
#undef fmodl

float fmodf(float x, float y){
	bool bits[3];
	return (float)__MCFCRT_fmod(&bits, x, y);
}
double fmod(double x, double y){
	bool bits[3];
	return (double)__MCFCRT_fmod(&bits, x, y);
}
long double fmodl(long double x, long double y){
	bool bits[3];
	return __MCFCRT_fmod(&bits, x, y);
}
