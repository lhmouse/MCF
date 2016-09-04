// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

extern float truncf(float x);
extern double trunc(double x);
extern long double truncl(long double x);

float roundf(float x){
	if(x < 0){
		return truncf(x - 0.5f);
	} else if(x > 0){
		return truncf(x + 0.5f);
	} else {
		return 0;
	}
}

double round(double x){
	if(x < 0){
		return trunc(x - 0.5);
	} else if(x > 0){
		return trunc(x + 0.5);
	} else {
		return 0;
	}
}

long double roundl(long double x){
	if(x < 0){
		return truncl(x - 0.5l);
	} else if(x > 0){
		return truncl(x + 0.5l);
	} else {
		return 0;
	}
}
