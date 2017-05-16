// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../env/expect.h"
#include <emmintrin.h>

#undef wmemcmp

int wmemcmp(const wchar_t *s1, const wchar_t *s2, size_t n){
	register const wchar_t *rp1 = s1;
	const wchar_t *const rend1 = rp1 + n;
	register const wchar_t *rp2 = s2;
	// 如果 rp 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	while(((uintptr_t)rp1 & 30) != 0){
#define CMP_GEN()	\
		{	\
			if(rp1 == rend1){	\
				return 0;	\
			}	\
			const int32_t rc1 = (uint16_t)*rp1;	\
			const int32_t rc2 = (uint16_t)*rp2;	\
			const int32_t d = rc1 - rc2;	\
			if(d != 0){	\
				return (d >> 31) | 1;	\
			}	\
			++rp1;	\
			++rp2;	\
		}
		CMP_GEN()
	}
	if((size_t)(rend1 - rp1) >= 64){
#define CMP_SSE2(load1_, load2_)	\
		{	\
			do {	\
				const __m128i xw10 = (load1_)((const __m128i *)rp1);	\
				const __m128i xw11 = (load1_)((const __m128i *)rp1 + 1);	\
				const __m128i xw20 = (load2_)((const __m128i *)rp2);	\
				const __m128i xw21 = (load2_)((const __m128i *)rp2 + 1);	\
				__m128i xt = _mm_packs_epi16(_mm_cmpeq_epi16(xw10, xw20), _mm_cmpeq_epi16(xw11, xw21));	\
				uint32_t mask = (uint16_t)~_mm_movemask_epi8(xt);	\
				if(_MCFCRT_EXPECT_NOT(mask != 0)){	\
					const int32_t tzne = __builtin_ctz(mask);	\
					const __m128i shift = _mm_set1_epi16(-0x8000);	\
					xt = _mm_packs_epi16(_mm_cmpgt_epi16(_mm_add_epi16(xw10, shift), _mm_add_epi16(xw20, shift)),	\
					                     _mm_cmpgt_epi16(_mm_add_epi16(xw11, shift), _mm_add_epi16(xw21, shift)));	\
					mask = (uint32_t)_mm_movemask_epi8(xt) | 0x80000000;	\
					const int32_t tzgt = __builtin_ctz(mask);	\
					const int32_t d = tzne - tzgt;	\
					return (d >> 31) | 1;	\
				}	\
				rp1 += 16;	\
				rp2 += 16;	\
			} while((size_t)(rend1 - rp1) >= 16);	\
		}
		if(((uintptr_t)rp2 & 15) == 0){
			CMP_SSE2(_mm_load_si128, _mm_load_si128)
		} else {
			CMP_SSE2(_mm_load_si128, _mm_loadu_si128)
		}
	}
	for(;;){
		CMP_GEN()
	}
}
