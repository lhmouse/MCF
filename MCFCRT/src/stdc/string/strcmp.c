// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../env/expect.h"
#include <pmmintrin.h>

#undef strcmp

int strcmp(const char *s1, const char *s2){
	register const char *rp1 = s1;
	register const char *rp2 = s2;
	// 如果 rp 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	while(((uintptr_t)rp1 & 31) != 0){
#define CMP_GEN()	\
		{	\
			const unsigned c1 = (uint8_t)*rp1;	\
			const unsigned c2 = (uint8_t)*rp2;	\
			if(c1 != c2){	\
				return (c1 < c2) ? -1 : 1;	\
			}	\
			if(c1 == 0){	\
				return 0;	\
			}	\
			++rp1;	\
			++rp2;	\
		}
		CMP_GEN()
	}
#define CMP_SSE3(load2_, care_about_page_boundaries_)	\
	{	\
		const __m128i xz = _mm_setzero_si128();	\
		uint8_t xmid = (uint8_t)((uintptr_t)rp2 / 32 * 2);	\
		for(;;){	\
			if(care_about_page_boundaries_){	\
				xmid = (uint8_t)(xmid + 2);	\
				if(_MCFCRT_EXPECT_NOT(xmid == 0)){	\
					char *const arp2 = (char *)((uintptr_t)rp2 & (uintptr_t)-32);	\
					const __m128i xw20 = _mm_load_si128((const __m128i *)arp2 + 0);	\
					const __m128i xw21 = _mm_load_si128((const __m128i *)arp2 + 1);	\
					__m128i xt = _mm_cmpeq_epi8(xw20, xz);	\
					uint32_t mask = (uint32_t)_mm_movemask_epi8(xt);	\
					xt = _mm_cmpeq_epi8(xw21, xz);	\
					mask += (uint32_t)_mm_movemask_epi8(xt) << 16;	\
					if(_MCFCRT_EXPECT_NOT(mask != 0)){	\
						break;	\
					}	\
				}	\
			}	\
			const __m128i xw10 = _mm_load_si128((const __m128i *)rp1 + 0);	\
			const __m128i xw11 = _mm_load_si128((const __m128i *)rp1 + 1);	\
			const __m128i xw20 = load2_((const __m128i *)rp2 + 0);	\
			const __m128i xw21 = load2_((const __m128i *)rp2 + 1);	\
			__m128i xt = _mm_cmpeq_epi8(xw10, xw20);	\
			uint32_t mask = (uint32_t)_mm_movemask_epi8(xt) ^ 0xFFFF;	\
			xt = _mm_cmpeq_epi8(xw11, xw21);	\
			mask += ((uint32_t)_mm_movemask_epi8(xt) ^ 0xFFFF) << 16;	\
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
			xt = _mm_cmpeq_epi8(xw10, xz);	\
			mask = (uint32_t)_mm_movemask_epi8(xt);	\
			xt = _mm_cmpeq_epi8(xw11, xz);	\
			mask += (uint32_t)_mm_movemask_epi8(xt) << 16;	\
			if(_MCFCRT_EXPECT_NOT(mask != 0)){	\
				return 0;	\
			}	\
			rp1 += 32;	\
			rp2 += 32;	\
		}	\
	}
	if(((uintptr_t)rp2 & 15) != 0){
		CMP_SSE3(_mm_lddqu_si128, true)
	} else if(((uintptr_t)rp2 & 63) != 0){
		CMP_SSE3(_mm_load_si128, true)
	} else {
		CMP_SSE3(_mm_load_si128, false)
	}
	for(;;){
		CMP_GEN()
	}
}
