// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

char *strchr(const char *s, int ch){
	register const char *rp = s;

	// 如果 rp 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	while(((uintptr_t)rp & (sizeof(uintptr_t) - 1)) != 0){
		register const char cur = *rp;
		if(cur == ch){
			return (char *)rp;
		} else if(cur == 0){
			return NULL;
		}
		++rp;
	}

	register uintptr_t full = (unsigned char)ch;
	for(size_t i = 1; i < sizeof(full); i <<= 1){
		full |= (full << (i * 8));
	}

	for(;;){

#ifdef _WIN64
#	define	MASK	0x0101010101010101ull
#else
#	define	MASK	0x01010101ul
#endif

#define UNROLLED(index)	\
		{	\
			register uintptr_t wrd = ((const uintptr_t *)rp)[(index)];	\
			register uintptr_t wrd2 = (wrd - MASK) & ~wrd;	\
			wrd ^= full;	\
			wrd = (wrd - MASK) & ~wrd;	\
			if(((wrd | wrd2) & (MASK << 7)) != 0){	\
				for(size_t i = 0; i < sizeof(uintptr_t) - 1; ++i){	\
					if((wrd & 0x80) != 0){	\
						return (char *)(rp + (index) * sizeof(uintptr_t) + i);	\
					}	\
					if((wrd2 & 0x80) != 0){	\
						return NULL;	\
					}	\
					wrd >>= 8;	\
					wrd2 >>= 8;	\
				}	\
				if((wrd & 0x80) != 0){	\
					return (char *)(rp + ((index) + 1) * sizeof(uintptr_t) - 1);	\
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

		rp += 8 * sizeof(uintptr_t);
	}
}
