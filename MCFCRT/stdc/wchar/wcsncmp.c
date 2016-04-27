// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

int wcsncmp(const wchar_t *s1, const wchar_t *s2, size_t n){
	size_t cnt = n;
	for(;;){

#define UNROLLED(idx_)	\
		{	\
			if(cnt == 0){	\
				return 0;	\
			}	\
			--cnt;	\
			const int_fast32_t ch1 = (int_fast32_t)(uint16_t)s1[idx_];	\
			const int_fast32_t ch2 = (int_fast32_t)(uint16_t)s2[idx_];	\
			const int_fast32_t delta = ch1 - ch2;	\
			if(delta != 0){	\
				return (delta >> (sizeof(delta) * __CHAR_BIT__ - 1)) | 1;	\
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
