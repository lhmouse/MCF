// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../string/_stringasm.h"

size_t wcslen(const wchar_t *s){
	register const wchar_t *rp = s;

	if(((uintptr_t)rp & 1) != 0){
		while(*rp != 0){
			++rp;
		}
		return (size_t)(rp - s);
	}

	// 如果 rp 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	while(((uintptr_t)rp & (sizeof(uintptr_t) - 1)) != 0){
		if(*rp == 0){
			return (size_t)(rp - s);
		}
		++rp;
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
			wrd = (wrd - MASK) & ~wrd & (MASK << 15);	\
			if(wrd != 0){	\
				for(size_t i = 0; i < sizeof(uintptr_t) / sizeof(wchar_t) - 1; ++i){	\
					if((wrd & 0x8000) != 0){	\
						return (size_t)(rp + (index) * sizeof(uintptr_t) / sizeof(wchar_t) + i - s);	\
					}	\
					wrd >>= 16;	\
				}	\
				return (size_t)(rp + ((index) + 1) * sizeof(uintptr_t) / sizeof(wchar_t) - 1 - s);	\
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
