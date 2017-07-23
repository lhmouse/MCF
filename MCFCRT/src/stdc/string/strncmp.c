// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../env/expect.h"
#include <pmmintrin.h>

#undef strncmp

int strncmp(const char *s1, const char *s2, size_t n){
	register const char *rp1 = s1;
	const char *const rend1 = rp1 + n;
	register const char *rp2 = s2;
	// 如果 rp 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	while(((uintptr_t)rp1 & 15) != 0){
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
			if(c1 == 0){	\
				return 0;	\
			}	\
			++rp1;	\
			++rp2;	\
		}
		CMP_GEN()
	}
	if((size_t)(rend1 - rp1) >= 64){
#define CMP_SSE3(load2_, care_about_page_boundaries_)	\
		{	\
			const __m128i xz = _mm_setzero_si128();	\
			uint8_t xmid = ((uintptr_t)rp2 >> 4) & 0xFF;	\
			do {	\
				if(care_about_page_boundaries_){	\
					xmid = (uint8_t)(xmid + 1);	\
					if(_MCFCRT_EXPECT_NOT(xmid == 0)){	\
						char *const arp2 = (char *)((uintptr_t)rp2 & (uintptr_t)-0x10);	\
						const __m128i xw2 = _mm_load_si128((const __m128i *)arp2);	\
						__m128i xt = _mm_cmpeq_epi8(xw2, xz);	\
						uint32_t mask = (uint32_t)_mm_movemask_epi8(xt);	\
						if(_MCFCRT_EXPECT_NOT(mask != 0)){	\
							break;	\
						}	\
					}	\
				}	\
				const __m128i xw1 = _mm_load_si128((const __m128i *)rp1);	\
				const __m128i xw2 = load2_((const __m128i *)rp2);	\
				__m128i xt = _mm_cmpeq_epi8(xw1, xw2);	\
				uint32_t mask = (uint16_t)~_mm_movemask_epi8(xt);	\
				if(_MCFCRT_EXPECT_NOT(mask != 0)){	\
					const int tzne = __builtin_ctzl(mask);	\
					const __m128i shift = _mm_set1_epi8(-0x80);	\
					xt = _mm_cmpgt_epi8(_mm_add_epi8(xw1, shift),	\
					                    _mm_add_epi8(xw2, shift));	\
					mask = (uint32_t)_mm_movemask_epi8(xt) | 0x80000000;	\
					const int tzgt = __builtin_ctzl(mask);	\
					return ((tzne - tzgt) >> 15) | 1;	\
				}	\
				xt = _mm_cmpeq_epi8(xw1, xz);	\
				mask = (uint32_t)_mm_movemask_epi8(xt);	\
				if(_MCFCRT_EXPECT_NOT(mask != 0)){	\
					return 0;	\
				}	\
				rp1 += 16;	\
				rp2 += 16;	\
			} while((size_t)(rend1 - rp1) >= 16);	\
		}
		if(((uintptr_t)rp2 & 15) == 0){
			CMP_SSE3(_mm_load_si128, false)
		} else {
			CMP_SSE3(_mm_lddqu_si128, true)
		}
	}
	for(;;){
		CMP_GEN()
	}
}
