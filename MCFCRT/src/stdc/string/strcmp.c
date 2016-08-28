// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../ext/expect.h"
#include <emmintrin.h>

int strcmp(const char *s1, const char *s2){
	register const char *rp1 = s1;
	register const char *rp2 = s2;
	// 如果 rp 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	while(((uintptr_t)rp1 & 15) != 0){
		const int32_t rc1 = (uint8_t)*rp1;
		const int32_t rc2 = (uint8_t)*rp2;
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
	if(((uintptr_t)rp2 & 15) == 0){
		const __m128i xz = _mm_setzero_si128();
		for(;;){
			const __m128i xw1 = _mm_load_si128((const __m128i *)rp1);
			const __m128i xw2 = _mm_load_si128((const __m128i *)rp2);
			__m128i xt = _mm_cmpeq_epi8(xw1, xw2);
			unsigned mask = (uint16_t)~_mm_movemask_epi8(xt);
			if(_MCFCRT_EXPECT_NOT(mask != 0)){
				const int32_t tzne = __builtin_ctz(mask);
				xt = _mm_cmplt_epi8(xw1, xw2);
				mask = (unsigned)_mm_movemask_epi8(xt);
				if(mask == 0){
					return 1;
				}
				const int32_t tzlt = __builtin_ctz(mask);
				const int32_t d = tzlt - tzne - 1;
				return (d >> 31) | 1;
			}
			xt = _mm_cmpeq_epi8(xw1, xz);
			mask = (unsigned)_mm_movemask_epi8(xt);
			if(_MCFCRT_EXPECT_NOT(mask != 0)){
				return 0;
			}
			rp1 += 16;
			rp2 += 16;
		}
	}
	for(;;){
		const int32_t rc1 = (uint8_t)*rp1;
		const int32_t rc2 = (uint8_t)*rp2;
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
