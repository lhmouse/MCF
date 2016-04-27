// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../env/_crtdef.h"
#include "../ext/assert.h"

#ifdef _WIN64
#	define MASK     0x0001000100010001ull
#else
#	define MASK     0x00010001ul
#endif

wchar_t *_MCFCRT_wcpcpy(wchar_t *restrict dst, const wchar_t *restrict src){
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

#define UNROLLED(idx_)	\
		{	\
			register uintptr_t wrd = ((const uintptr_t *)rp)[(idx_)];	\
			if(((wrd - MASK) & ~wrd & (MASK << 15)) != 0){	\
				wp += (idx_) * sizeof(uintptr_t) / sizeof(wchar_t);	\
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

		rp += 8 * sizeof(uintptr_t) / sizeof(wchar_t);
		wp += 8 * sizeof(uintptr_t) / sizeof(wchar_t);
	}
}
wchar_t *_MCFCRT_wcppcpy(wchar_t *restrict dst, wchar_t *restrict end, const wchar_t *restrict src){
	register const wchar_t *rp = src;
	register wchar_t *wp = dst;

	_MCFCRT_ASSERT_MSG(wp < end, L"目的缓冲区至少应为一个字符大小以容纳字符串结束符。");

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
			if((size_t)(end - 1 - wp) < ((idx_) + 1) * sizeof(uintptr_t) / sizeof(wchar_t)){	\
				wp += (idx_) * sizeof(uintptr_t) / sizeof(wchar_t);	\
				for(size_t i = 0; i < sizeof(uintptr_t) / sizeof(wchar_t) - 1; ++i){	\
					if(wp >= end - 1){	\
						*wp = 0;	\
						return wp;	\
					}	\
					if((*wp = (wchar_t)(uint16_t)(wrd & 0xFFFF)) == 0){	\
						return wp;	\
					}	\
					++wp;	\
					wrd >>= 16;	\
				}	\
				*wp = 0;	\
				return wp;	\
			}	\
			if(((wrd - MASK) & ~wrd & (MASK << 15)) != 0){	\
				wp += (idx_) * sizeof(uintptr_t) / sizeof(wchar_t);	\
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

		rp += 8 * sizeof(uintptr_t) / sizeof(wchar_t);
		wp += 8 * sizeof(uintptr_t) / sizeof(wchar_t);
	}
}
