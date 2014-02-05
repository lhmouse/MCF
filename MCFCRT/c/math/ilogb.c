// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

extern float logbf(float x);
extern double logb(double x);
extern long double logbl(long double x);

int ilogbf(float x){
	return (int)logbf(x);
}

int ilogb(double x){
	return (int)logb(x);
}

int ilogbl(long double x){
	return (int)logbl(x);
}
