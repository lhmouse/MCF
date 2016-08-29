// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../ext/expect.h"
#include <emmintrin.h>

wchar_t *wmemchr(const wchar_t *s, wchar_t c, size_t n){
	register const wchar_t *rp = s;
	const wchar_t *const rend = rp + n;
	// 如果 rp 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	while(((uintptr_t)rp & 30) != 0){
		if(rp == rend){
			return nullptr;
		}
		const wchar_t rc = *rp;
		if(rc == c){
			return (wchar_t *)rp;
		}
		++rp;
	}
	if((size_t)(rend - rp) >= 64){
		const __m128i xc = _mm_set1_epi16((int16_t)c);
		do {
			const __m128i xw0 = _mm_load_si128((const __m128i *)rp);
			const __m128i xw1 = _mm_load_si128((const __m128i *)rp + 1);
			__m128i xt = _mm_packs_epi16(_mm_cmpeq_epi16(xw0, xc), _mm_cmpeq_epi16(xw1, xc));
			uint32_t mask = (uint32_t)_mm_movemask_epi8(xt);
			if(_MCFCRT_EXPECT_NOT(mask != 0)){
				return (wchar_t *)rp + __builtin_ctz(mask);
			}
			rp += 16;
		} while((size_t)(rend - rp) >= 16);
	}
	for(;;){
		if(rp == rend){
			return nullptr;
		}
		const wchar_t rc = *rp;
		if(rc == c){
			return (wchar_t *)rp;
		}
		++rp;
	}
}
