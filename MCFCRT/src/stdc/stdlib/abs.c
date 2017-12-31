// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

#undef abs
#undef labs
#undef llabs

#define SET_SIGN_MASK(r_)    do { (r_) >>= (sizeof((r_)) * CHAR_BIT - 1); } while(false)

int abs(int i){
	int m = i;
	SET_SIGN_MASK(m);
	return (i ^ m) - m;
}
long labs(long i){
	long m = i;
	SET_SIGN_MASK(m);
	return (i ^ m) - m;
}
long long llabs(long long i){
	long long m = i;
	SET_SIGN_MASK(m);
	return (i ^ m) - m;
}
