// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

int strcmp(const char *s1, const char *s2){
	for(;;){

#define UNROLLED(index)	\
		{	\
			const char ch1 = s1[index];	\
			const int delta = (int)(unsigned char)ch1 - (int)(unsigned char)s2[index];	\
			if(delta != 0){	\
				return (delta >> (sizeof(int) * CHAR_BIT - 1)) | 1;	\
			}	\
			if(ch1 == 0){	\
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
