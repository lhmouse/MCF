// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

__MCF_CRT_EXTERN int abs(int i){
	const unsigned int mask = (unsigned int)(i >> (sizeof(int) * 8 - 1));
	return (int)(((unsigned int)i ^ mask) - mask);
}

__MCF_CRT_EXTERN long labs(long i){
	const unsigned long mask = (unsigned long)(i >> (sizeof(long) * 8 - 1));
	return (long)(((unsigned long)i ^ mask) - mask);
}

__MCF_CRT_EXTERN long long llabs(long long i){
	const unsigned long long mask = (unsigned long long)(i >> (sizeof(long long) * 8 - 1));
	return (long long)(((unsigned long long)i ^ mask) - mask);
}
