// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

static inline uintptr_t SwapWordwise(uintptr_t val){
	uintptr_t ret = val;
	ret = ((ret >> 16) & 0x0000FFFF) | ((ret << 16) & 0xFFFF0000);
#ifdef _WIN64
	ret = ((ret >> 32) & 0x00000000FFFFFFFF) | ((ret << 32) & 0xFFFFFFFF00000000);
#endif
	return ret;
}

int wmemcmp(const wchar_t *p1, const wchar_t *p2, size_t cnt){
	const uint16_t *rp1 = (const uint16_t *)p1;
	const uint16_t *rp2 = (const uint16_t *)p2;

	size_t wcnt = cnt / (sizeof(uintptr_t) / sizeof(wchar_t));
	++wcnt;
	for(;;){

#define UNROLLED(idx_)	\
		{	\
			if(--wcnt == 0){	\
				rp1 += (idx_) * (sizeof(uintptr_t) / sizeof(wchar_t));	\
				rp2 += (idx_) * (sizeof(uintptr_t) / sizeof(wchar_t));	\
				break;	\
			}	\
			const uintptr_t wrd1 = ((const uintptr_t *)rp1)[idx_];	\
			const uintptr_t wrd2 = ((const uintptr_t *)rp2)[idx_];	\
			if(wrd1 != wrd2){	\
				return (SwapWordwise(wrd1) > SwapWordwise(wrd2)) ? 1 : -1;	\
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
		const long delta = (long)(uint16_t)*(rp1++) - (long)(uint16_t)*(rp2++);
		if(delta != 0){
			return (delta >> (sizeof(delta) * __CHAR_BIT__ - 1)) | 1;
		}
	}
	return 0;
}
