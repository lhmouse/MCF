// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

wchar_t *_wcscpyout(wchar_t *restrict dst, const wchar_t *restrict src){
	register const wchar_t *rp = src;
	register wchar_t *wp = dst;

	if(((uintptr_t)rp & 1) != 0){
		for(;;){
			if((*wp = *(rp++)) == 0){
				return wp;
			}
			++wp;
		}
	}

	// 如果 rp 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	while(((uintptr_t)rp & (sizeof(uintptr_t) - 1)) != 0){
		if((*wp = *(rp++)) == 0){
			return wp;
		}
		++wp;
	}

	for(;;){

#ifdef _WIN64
#	define	MASK	0x0001000100010001ull
#else
#	define	MASK	0x00010001ul
#endif

#define UNROLLED(index)	\
		{	\
			register uintptr_t wrd = ((const uintptr_t *)rp)[(index)];	\
			if(((wrd - MASK) & ~wrd & (MASK << 15)) != 0){	\
				wp += (index) * sizeof(uintptr_t) / sizeof(wchar_t);	\
				for(size_t i = 0; i < sizeof(uintptr_t) / sizeof(wchar_t) - 1; ++i){	\
					if((*wp = (wchar_t)(uint16_t)(wrd & 0xFFFF)) == 0){	\
						return wp;	\
					}	\
					++wp;	\
					wrd >>= 16;	\
				}	\
				*wp = 0;	\
				return wp;	\
			}	\
			((uintptr_t *)wp)[(index)] = wrd;	\
		}

		UNROLLED(0)
		UNROLLED(1)
		UNROLLED(2)
		UNROLLED(3)
		UNROLLED(4)
		UNROLLED(5)
		UNROLLED(6)
		UNROLLED(7)

		rp += 8 * sizeof(uintptr_t) / sizeof(wchar_t);
		wp += 8 * sizeof(uintptr_t) / sizeof(wchar_t);
	}
}
