// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

wchar_t *wcschr(const wchar_t *s, wchar_t ch){
	register const wchar_t *rp = s;

	if(((uintptr_t)rp & 1) != 0){
		for(;;){
			register const wchar_t cur = *rp;
			if(cur == ch){
				return (wchar_t *)rp;
			} else if(cur == 0){
				return NULL;
			}
			++rp;
		}
	}

	// 如果 rp 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	while(((uintptr_t)rp & (sizeof(uintptr_t) - 1)) != 0){
		register const wchar_t cur = *rp;
		if(cur == ch){
			return (wchar_t *)rp;
		} else if(cur == 0){
			return NULL;
		}
		++rp;
	}

	register uintptr_t full = (uint16_t)ch;
	for(size_t i = 2; i < sizeof(full); i <<= 1){
		full |= (full << (i * 8));
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
			register uintptr_t wrd2 = (wrd - MASK) & ~wrd;	\
			wrd ^= full;	\
			wrd = (wrd - MASK) & ~wrd;	\
			if(((wrd | wrd2) & (MASK << 15)) != 0){	\
				for(size_t i = 0; i < sizeof(uintptr_t) / sizeof(wchar_t) - 1; ++i){	\
					if((wrd & 0x8000) != 0){	\
						return (wchar_t *)(rp + (index) * sizeof(uintptr_t) / sizeof(wchar_t) + i);	\
					}	\
					if((wrd2 & 0x8000) != 0){	\
						return NULL;	\
					}	\
					wrd >>= 16;	\
					wrd2 >>= 16;	\
				}	\
				if((wrd & 0x8000) != 0){	\
					return (wchar_t *)(rp + ((index) + 1) * sizeof(uintptr_t) / sizeof(wchar_t) - 1);	\
				}	\
				return NULL;	\
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

		rp += 8 * sizeof(uintptr_t) / sizeof(wchar_t);
	}
}
