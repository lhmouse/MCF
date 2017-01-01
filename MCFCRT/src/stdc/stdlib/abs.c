// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

#undef abs
#undef labs
#undef llabs

int abs(int i){
	register const unsigned mask = (unsigned)(i >> (sizeof(int) * CHAR_BIT - 1));
	return (int)(((unsigned)i ^ mask) - mask);
}
long labs(long i){
	register const unsigned long mask = (unsigned long)(i >> (sizeof(long) * CHAR_BIT - 1));
	return (long)(((unsigned long)i ^ mask) - mask);
}
long long llabs(long long i){
	register const unsigned long long mask = (unsigned long long)(i >> (sizeof(long long) * CHAR_BIT - 1));
	return (long long)(((unsigned long long)i ^ mask) - mask);
}
