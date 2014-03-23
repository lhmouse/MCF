// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

wchar_t *wmemchr(const wchar_t *s, wchar_t ch, size_t cnt){
	register const wchar_t *rp = s;
	const wchar_t *const end = rp + cnt;

	if((((uintptr_t)rp & (sizeof(wchar_t) - 1)) == 0) && (cnt * sizeof(wchar_t) >= sizeof(uintptr_t) * 2)){
		while(((uintptr_t)rp & (sizeof(uintptr_t) - 1)) != 0){
			if(*rp == ch){
				return (wchar_t *)rp;
			}
			++rp;
		}

		register uintptr_t full = (uint16_t)ch;
		for(size_t i = 2; i < sizeof(full); i <<= 1){
			full |= (full << (i * 8));
		}

		const wchar_t *const wend = (const wchar_t *)((uintptr_t)end & ~(sizeof(uintptr_t) - 1));
		for(;;){

#ifdef _WIN64
#	define	MASK	0x0001000100010001ull
#else
#	define	MASK	0x00010001ul
#endif

#define UNROLLED	\
			{	\
				register uintptr_t wrd = (*(const uintptr_t *)rp) ^ full;	\
				wrd = (wrd - MASK) & ~wrd;	\
				if((wrd & (MASK << 15)) != 0){	\
					for(size_t i = 0; i < sizeof(uintptr_t) / sizeof(wchar_t) - 1; ++i){	\
						if((wrd & 0x8000) != 0){	\
							return (wchar_t *)(rp + i);	\
						}	\
						wrd >>= 16;	\
					}	\
					return (wchar_t *)(rp + sizeof(uintptr_t) / sizeof(wchar_t) - 1);	\
				}	\
				rp += sizeof(uintptr_t) / sizeof(wchar_t);	\
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
		if(*rp == ch){
			return (wchar_t *)rp;
		}
		++rp;
	}
	return NULL;
}
