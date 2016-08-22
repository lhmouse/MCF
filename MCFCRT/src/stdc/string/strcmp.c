// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

int strcmp(const char *s1, const char *s2){
	for(;;){

#define UNROLLED(idx_)	\
		{	\
			const int32_t c1 = (unsigned char)s1[idx_];	\
			const int32_t c2 = (unsigned char)s2[idx_];	\
			const int32_t delta = c1 - c2;	\
			if(delta != 0){	\
				return (delta >> 31) | 1;	\
			}	\
			if(c1 == 0){	\
				return 0;	\
			}	\
		}

		UNROLLED(0)
		UNROLLED(1)
		UNROLLED(2)
		UNROLLED(3)
		UNROLLED(4)
		UNROLLED(5)
		UNROLLED(6)
		UNROLLED(7)

		s1 += 8;
		s2 += 8;
	}
}
