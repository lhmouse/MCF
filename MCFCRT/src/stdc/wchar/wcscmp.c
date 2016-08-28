// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../ext/expect.h"
#include <emmintrin.h>

int wcscmp(const wchar_t *s1, const wchar_t *s2){
	register const wchar_t *rp1 = s1;
	register const wchar_t *rp2 = s2;
	// 如果 rp 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	while(((uintptr_t)rp1 & 31) != 0){
		const int32_t rc1 = (uint16_t)*rp1;
		const int32_t rc2 = (uint16_t)*rp2;
		const int32_t d = rc1 - rc2;
		if(d != 0){
			return (d >> 31) | 1;
		}
		if(rc1 == 0){
			return 0;
		}
		++rp1;
		++rp2;
	}
#define SSE2_CMP(load1_, load2_)	\
	{	\
		const __m128i xz = _mm_setzero_si128();	\
		for(;;){	\
			const __m128i xw01 = (load1_)((const __m128i *)rp1);	\
			const __m128i xw02 = (load2_)((const __m128i *)rp2);	\
			const __m128i xw11 = (load1_)((const __m128i *)rp1 + 1);	\
			const __m128i xw12 = (load2_)((const __m128i *)rp2 + 1);	\
			__m128i xt = _mm_packs_epi16(_mm_cmpeq_epi16(xw01, xw02), _mm_cmpeq_epi16(xw11, xw12));	\
			unsigned mask = (uint16_t)~_mm_movemask_epi8(xt);	\
			if(_MCFCRT_EXPECT_NOT(mask != 0)){	\
				const int32_t tzne = __builtin_ctz(mask);	\
				const __m128i shift = _mm_set1_epi16(-0x8000);	\
				xt = _mm_packs_epi16(_mm_cmpgt_epi16(_mm_add_epi16(xw01, shift), _mm_add_epi16(xw02, shift)),	\
				                     _mm_cmpgt_epi16(_mm_add_epi16(xw11, shift), _mm_add_epi16(xw12, shift)));	\
				mask = (unsigned)_mm_movemask_epi8(xt);	\
				if(mask == 0){	\
					return -1;	\
				}	\
				const int32_t tzgt = __builtin_ctz(mask);	\
				const int32_t d = tzne - tzgt;	\
				return (d >> 31) | 1;	\
			}	\
			xt = _mm_packs_epi16(_mm_cmpeq_epi16(xw01, xz), _mm_cmpeq_epi16(xw11, xz));	\
			mask = (unsigned)_mm_movemask_epi8(xt);	\
			if(_MCFCRT_EXPECT_NOT(mask != 0)){	\
				return 0;	\
			}	\
			rp1 += 16;	\
			rp2 += 16;	\
		}	\
	}
	if(((uintptr_t)rp2 & 31) == 0){
		SSE2_CMP(_mm_load_si128, _mm_load_si128)
	} else {
//		SSE2_CMP(_mm_load_si128, _mm_loadu_si128)
		for(;;){
			const int32_t rc1 = (uint16_t)*rp1;
			const int32_t rc2 = (uint16_t)*rp2;
			const int32_t d = rc1 - rc2;
			if(d != 0){
				return (d >> 31) | 1;
			}
			if(rc1 == 0){
				return 0;
			}
			++rp1;
			++rp2;
		}
	}
}
