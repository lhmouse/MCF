// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

void *memchr(const void *s, int ch, size_t cb){
	register const unsigned char *rp = (const unsigned char *)s;
	const unsigned char *const end = rp + cb;

	if(cb >= sizeof(uintptr_t) * 2){
		while(((uintptr_t)rp & (sizeof(uintptr_t) - 1)) != 0){
			if(*rp == (unsigned char)ch){
				return (void *)rp;
			}
			++rp;
		}

		register uintptr_t full = (unsigned char)ch;
		for(size_t i = 1; i < sizeof(full); i <<= 1){
			full |= (full << (i * 8));
		}

		const unsigned char *const wend = (const unsigned char *)((uintptr_t)end & ~(sizeof(uintptr_t) - 1));
		for(;;){

#ifdef _WIN64
#	define	MASK	0x0101010101010101ull
#else
#	define	MASK	0x01010101ul
#endif

#define UNROLLED	\
			{	\
				register uintptr_t wrd = (*(const uintptr_t *)rp) ^ full;	\
				wrd = (wrd - MASK) & ~wrd;	\
				if((wrd & (MASK << 7)) != 0){	\
					for(size_t i = 0; i < sizeof(uintptr_t) - 1; ++i){	\
						if((wrd & 0x80) != 0){	\
							return (void *)(rp + i);	\
						}	\
						wrd >>= 8;	\
					}	\
					return (void *)(rp + sizeof(uintptr_t) - 1);	\
				}	\
				rp += sizeof(uintptr_t);	\
				if(rp == wend){	\
					break;	\
				}	\
			}

			UNROLLED
			UNROLLED
			UNROLLED
			UNROLLED
			UNROLLED
			UNROLLED
			UNROLLED
			UNROLLED
		}
	}

	while(rp != end){
		if(*rp == (unsigned char)ch){
			return (void *)rp;
		}
		++rp;
	}
	return NULL;
}
