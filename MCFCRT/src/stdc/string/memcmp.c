// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../env/expect.h"
#include <pmmintrin.h>

#undef memcmp

int memcmp(const void *s1, const void *s2, size_t n){
	register const char *rp1 = s1;
	const char *const rend1 = rp1 + n;
	register const char *rp2 = s2;
	// 如果 rp 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	while(((uintptr_t)rp1 & 31) != 0){
#define CMP_GEN()	\
		{	\
			if(rp1 == rend1){	\
				return 0;	\
			}	\
			const unsigned c1 = (uint8_t)*rp1;	\
			const unsigned c2 = (uint8_t)*rp2;	\
			if(c1 != c2){	\
				return (c1 < c2) ? -1 : 1;	\
			}	\
			++rp1;	\
			++rp2;	\
		}
		CMP_GEN()
	}
	if((size_t)(rend1 - rp1) >= 64){
#define CMP_SSE3(load2_)	\
		{	\
			do {	\
				const __m128i xw10 = _mm_load_si128((const __m128i *)rp1 + 0);	\
				const __m128i xw11 = _mm_load_si128((const __m128i *)rp1 + 1);	\
				const __m128i xw20 = load2_((const __m128i *)rp2 + 0);	\
				const __m128i xw21 = load2_((const __m128i *)rp2 + 1);	\
				__m128i xt = _mm_cmpeq_epi8(xw10, xw20);	\
				uint32_t mask = (uint16_t)~_mm_movemask_epi8(xt);	\
				xt = _mm_cmpeq_epi8(xw11, xw21);	\
				mask += (uint32_t)~_mm_movemask_epi8(xt) << 16;	\
				if(_MCFCRT_EXPECT_NOT(mask != 0)){	\
					const int tzne = __builtin_ctzl(mask);	\
					const __m128i shift = _mm_set1_epi8(-0x80);	\
					xt = _mm_cmpgt_epi8(_mm_add_epi8(xw10, shift),	\
					                    _mm_add_epi8(xw20, shift));	\
					mask = (uint32_t)_mm_movemask_epi8(xt);	\
					xt = _mm_cmpgt_epi8(_mm_add_epi8(xw11, shift),	\
					                    _mm_add_epi8(xw21, shift));	\
					mask += (uint32_t)_mm_movemask_epi8(xt) << 16;	\
					const int tzgt = (mask == 0) ? 32 : __builtin_ctzl(mask);	\
					return ((tzne - tzgt) >> 15) | 1;	\
				}	\
				rp1 += 32;	\
				rp2 += 32;	\
			} while((size_t)(rend1 - rp1) >= 32);	\
		}
		if(((uintptr_t)rp2 & 15) == 0){
			CMP_SSE3(_mm_load_si128)
		} else {
			CMP_SSE3(_mm_lddqu_si128)
		}
	}
	for(;;){
		CMP_GEN()
	}
}
