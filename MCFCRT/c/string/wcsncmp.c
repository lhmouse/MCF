// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

int wcsncmp(const wchar_t *s1, const wchar_t *s2, size_t n){
	size_t cnt = n;
	++cnt;
	for(;;){

#define UNROLLED(index)	\
		{	\
			if(--cnt == 0){	\
				return 0;	\
			}	\
			const int ch1 = (uint16_t)s1[index];	\
			const int ch2 = (uint16_t)s2[index];	\
			if(ch1 != ch2){	\
				return (ch1 > ch2) ? 1 : -1;	\
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
