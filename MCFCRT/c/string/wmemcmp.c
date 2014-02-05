// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

int wmemcmp(const wchar_t *p1, const wchar_t *p2, size_t cnt){
	const uint16_t *rp1 = (const uint16_t *)p1;
	const uint16_t *rp2 = (const uint16_t *)p2;

	size_t wcnt = cnt / (sizeof(uintptr_t) / sizeof(wchar_t));
	++wcnt;
	for(;;){

#define COMPARE_LOWORD_AND_SHIFT	\
		{	\
			const int delta = (int)(uint16_t)wrd1 - (int)(uint16_t)wrd2;	\
			if(delta != 0){	\
				return (delta >> (sizeof(int) * CHAR_BIT - 1)) | 1;	\
			}	\
			wrd1 >>= 16;	\
			wrd2 >>= 16;	\
		}

#ifdef _WIN64
#	define	COMPARE_UINTPTR	\
		COMPARE_LOWORD_AND_SHIFT	\
		COMPARE_LOWORD_AND_SHIFT	\
		COMPARE_LOWORD_AND_SHIFT	\
		COMPARE_LOWORD_AND_SHIFT
#else
#	define	COMPARE_UINTPTR	\
		COMPARE_LOWORD_AND_SHIFT	\
		COMPARE_LOWORD_AND_SHIFT
#endif

#define UNROLLED(index)	\
		{	\
			if(--wcnt == 0){	\
				rp1 += (index) * (sizeof(uintptr_t) / sizeof(wchar_t));	\
				rp2 += (index) * (sizeof(uintptr_t) / sizeof(wchar_t));	\
				break;	\
			}	\
			register uintptr_t wrd1 = ((const uintptr_t *)rp1)[index];	\
			register uintptr_t wrd2 = ((const uintptr_t *)rp2)[index];	\
			if(wrd1 != wrd2){	\
				COMPARE_UINTPTR	\
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

		rp1 += 8 * (sizeof(uintptr_t) / sizeof(wchar_t));
		rp2 += 8 * (sizeof(uintptr_t) / sizeof(wchar_t));
	}
	size_t rem = cnt % (sizeof(uintptr_t) / sizeof(wchar_t));
	while(rem-- != 0){
		const int delta = (int)(uint16_t)*(rp1++) - (int)(uint16_t)*(rp2++);
		if(delta != 0){
			return (delta >> (sizeof(int) * CHAR_BIT - 1)) | 1;
		}
	}
	return 0;
}
