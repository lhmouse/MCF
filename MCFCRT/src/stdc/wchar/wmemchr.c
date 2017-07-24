// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../env/expect.h"
#include <pmmintrin.h>

#undef wmemchr

wchar_t *wmemchr(const wchar_t *s, wchar_t c, size_t n){
	register const wchar_t *rp = s;
	const wchar_t *const rend = rp + n;
	// 如果 rp 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	while(((uintptr_t)rp & 63) != 0){
#define CHR_GEN()	\
		{	\
			if(rp == rend){	\
				return _MCFCRT_NULLPTR;	\
			}	\
			const wchar_t rc = *rp;	\
			if(rc == c){	\
				return (wchar_t *)rp;	\
			}	\
			++rp;	\
		}
		CHR_GEN()
	}
	if((size_t)(rend - rp) >= 64){
#define CHR_SSE3()	\
		{	\
			const __m128i xc = _mm_set1_epi16((int16_t)c);	\
			do {	\
				const __m128i xw0 = _mm_load_si128((const __m128i *)rp + 0);	\
				const __m128i xw1 = _mm_load_si128((const __m128i *)rp + 1);	\
				const __m128i xw2 = _mm_load_si128((const __m128i *)rp + 2);	\
				const __m128i xw3 = _mm_load_si128((const __m128i *)rp + 3);	\
				__m128i xt = _mm_packs_epi16(_mm_cmpeq_epi16(xw0, xc),	\
				                             _mm_cmpeq_epi16(xw1, xc));	\
				uint32_t mask = (uint32_t)_mm_movemask_epi8(xt);	\
				xt = _mm_packs_epi16(_mm_cmpeq_epi16(xw2, xc),	\
				                     _mm_cmpeq_epi16(xw3, xc));	\
				mask += (uint32_t)_mm_movemask_epi8(xt) << 16;	\
				if(_MCFCRT_EXPECT_NOT(mask != 0)){	\
					return (wchar_t *)rp + __builtin_ctzl(mask);	\
				}	\
				rp += 32;	\
			} while((size_t)(rend - rp) >= 32);	\
		}
		CHR_SSE3()
	}
	for(;;){
		CHR_GEN()
	}
}
