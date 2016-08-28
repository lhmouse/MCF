// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../ext/expect.h"
#include <emmintrin.h>

int wcsncmp(const wchar_t *s1, const wchar_t *s2, size_t n){
	register const wchar_t *rp1 = s1;
	const wchar_t *const rend1 = rp1 + n;
	register const wchar_t *rp2 = s2;
	// 如果 rp 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	while(((uintptr_t)rp1 & 31) != 0){
		if(rp1 == rend1){
			return 0;
		}
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
	if((((uintptr_t)rp2 & 31) == 0) && ((size_t)(rend1 - rp1) >= 64)){
		const __m128i xz = _mm_setzero_si128();
		do {
			const __m128i xw01 = _mm_load_si128((const __m128i *)rp1);
			const __m128i xw02 = _mm_load_si128((const __m128i *)rp2);
			const __m128i xw11 = _mm_load_si128((const __m128i *)rp1 + 1);
			const __m128i xw12 = _mm_load_si128((const __m128i *)rp2 + 1);
			__m128i xt = _mm_packs_epi16(_mm_cmpeq_epi16(xw01, xw02), _mm_cmpeq_epi16(xw11, xw12));
			unsigned mask = (uint16_t)~_mm_movemask_epi8(xt);
			if(_MCFCRT_EXPECT_NOT(mask != 0)){
				const int32_t tzne = __builtin_ctz(mask);
				xt = _mm_packs_epi16(_mm_cmplt_epi16(xw01, xw02), _mm_cmplt_epi16(xw11, xw12));
				mask = (unsigned)_mm_movemask_epi8(xt);
				const int32_t tzlt = __builtin_ctz(mask);
				const int32_t d = tzlt - tzne - 1;
				return (d >> 31) | 1;
			}
			xt = _mm_packs_epi16(_mm_cmpeq_epi16(xw01, xz), _mm_cmpeq_epi16(xw11, xz));
			mask = (unsigned)_mm_movemask_epi8(xt);
			if(_MCFCRT_EXPECT_NOT(mask != 0)){
				return 0;
			}
			rp1 += 16;
			rp2 += 16;
		} while((size_t)(rend1 - rp1) >= 16);
	}
	for(;;){
		if(rp1 == rend1){
			return 0;
		}
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
