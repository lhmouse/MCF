// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

__MCF_CRT_EXTERN float sqrtf(float x);
__MCF_CRT_EXTERN double sqrt(double x);
__MCF_CRT_EXTERN long double sqrtl(long double x);

__MCF_CRT_EXTERN float hypotf(float x, float y){
	return sqrtf(x * x + y * y);
}

__MCF_CRT_EXTERN double hypot(double x, double y){
	return sqrt(x * x + y * y);
}

__MCF_CRT_EXTERN long double hypotl(long double x, long double y){
	return sqrtl(x * x + y * y);
}
