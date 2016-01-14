// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../env/_crtdef.h"
#include "../env/bail.h"

#ifdef _WIN64
#	define MASK     0x0101010101010101ull
#else
#	define MASK     0x01010101ul
#endif

char *MCFCRT_stpcpy(char *restrict dst, const char *restrict src){
	register const char *rp = src;
	register char *wp = dst;

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

#define UNROLLED(idx_)	\
		{	\
			register uintptr_t wrd = ((const uintptr_t *)rp)[(idx_)];	\
			if(((wrd - MASK) & ~wrd & (MASK << 7)) != 0){	\
				wp += (idx_) * sizeof(uintptr_t);	\
				for(size_t i = 0; i < sizeof(uintptr_t) - 1; ++i){	\
					if((*wp = (char)(unsigned char)(wrd & 0xFF)) == 0){	\
						return wp;	\
					}	\
					++wp;	\
					wrd >>= 8;	\
				}	\
				*wp = 0;	\
				return wp;	\
			}	\
			((uintptr_t *)wp)[(idx_)] = wrd;	\
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
		wp += 8 * sizeof(uintptr_t);
	}
}
char *MCFCRT_stppcpy(char *restrict dst, char *restrict end, const char *restrict src){
	register const char *rp = src;
	register char *wp = dst;

	if(wp == end){
		MCFCRT_Bail(L"目的缓冲区至少应为一个字符大小以容纳字符串结束符。");
	}

	// 如果 rp 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	while(((uintptr_t)rp & (sizeof(uintptr_t) - 1)) != 0){
		if(wp >= end - 1){
			*wp = 0;
			return wp;
		}
		if((*wp = *(rp++)) == 0){
			return wp;
		}
		++wp;
	}

	for(;;){

#define UNROLLED_P(idx_)	\
		{	\
			register uintptr_t wrd = ((const uintptr_t *)rp)[(idx_)];	\
			if((size_t)(end - 1 - wp) < ((idx_) + 1) * sizeof(uintptr_t)){	\
				wp += (idx_) * sizeof(uintptr_t);	\
				for(size_t i = 0; i < sizeof(uintptr_t) - 1; ++i){	\
					if(wp >= end - 1){	\
						*wp = 0;	\
						return wp;	\
					}	\
					if((*wp = (char)(unsigned char)(wrd & 0xFF)) == 0){	\
						return wp;	\
					}	\
					++wp;	\
					wrd >>= 8;	\
				}	\
				*wp = 0;	\
				return wp;	\
			}	\
			if(((wrd - MASK) & ~wrd & (MASK << 7)) != 0){	\
				wp += (idx_) * sizeof(uintptr_t);	\
				for(size_t i = 0; i < sizeof(uintptr_t) - 1; ++i){	\
					if((*wp = (char)(unsigned char)(wrd & 0xFF)) == 0){	\
						return wp;	\
					}	\
					++wp;	\
					wrd >>= 8;	\
				}	\
				*wp = 0;	\
				return wp;	\
			}	\
			((uintptr_t *)wp)[(idx_)] = wrd;	\
		}

		UNROLLED_P(0)
		UNROLLED_P(1)
		UNROLLED_P(2)
		UNROLLED_P(3)
		UNROLLED_P(4)
		UNROLLED_P(5)
		UNROLLED_P(6)
		UNROLLED_P(7)

		rp += 8 * sizeof(uintptr_t);
		wp += 8 * sizeof(uintptr_t);
	}
}
