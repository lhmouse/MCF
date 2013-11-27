// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

__MCF_CRT_EXTERN float logbf(float x);
__MCF_CRT_EXTERN double logb(double x);
__MCF_CRT_EXTERN long double logbl(long double x);

__MCF_CRT_EXTERN int ilogbf(float x){
	return (int)logbf(x);
}

__MCF_CRT_EXTERN int ilogb(double x){
	return (int)logb(x);
}

__MCF_CRT_EXTERN int ilogbl(long double x){
	return (int)logbl(x);
}
