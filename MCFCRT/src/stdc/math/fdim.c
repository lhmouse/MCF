// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef fdimf
#undef fdim
#undef fdiml

float fdimf(float x, float y){
	return (x > y) ? (x - y) : +0.0f;
}
double fdim(double x, double y){
	return (x > y) ? (x - y) : +0.0;
}
long double fdiml(long double x, long double y){
	return (x > y) ? (x - y) : +0.0l;
}
