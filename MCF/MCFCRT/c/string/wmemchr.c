// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

__MCF_CRT_EXTERN wchar_t *wmemchr(const wchar_t *s, wchar_t ch, size_t cnt){
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
		do {

#ifdef __amd64__
#	define	MASK	0x0001000100010001ull
#else
#	define	MASK	0x00010001ul
#endif

#define UNROLLED(index)	\
			{	\
				register uintptr_t wrd = ((const uintptr_t *)rp)[(index)] ^ full;	\
				wrd = (wrd - MASK) & ~wrd;	\
				if((wrd & (MASK << 15)) != 0){	\
					for(size_t i = 0; i < sizeof(uintptr_t) / sizeof(wchar_t) - 1; ++i){	\
						if((wrd & 0x8000) != 0){	\
							return (wchar_t *)(rp + (index) * sizeof(uintptr_t) / sizeof(wchar_t) + i);	\
						}	\
						wrd >>= 16;	\
					}	\
					return (wchar_t *)(rp + ((index) + 1) * sizeof(uintptr_t) / sizeof(wchar_t) - 1);	\
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
		} while(rp != wend);
	}

	while(rp != end){
		if(*rp == ch){
			return (wchar_t *)rp;
		}
		++rp;
	}
	return NULL;
}
