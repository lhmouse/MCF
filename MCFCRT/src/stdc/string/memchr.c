// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../ext/expect.h"
#include <emmintrin.h>

#undef memchr

void *memchr(const void *s, int c, size_t n){
	register const char *rp = s;
	const char *const rend = rp + n;
	// 如果 rp 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	while(((uintptr_t)rp & 15) != 0){
		if(rp == rend){
			return nullptr;
		}
		const char rc = *rp;
		if(rc == (char)c){
			return (char *)rp;
		}
		++rp;
	}
	if((size_t)(rend - rp) >= 64){
		const __m128i xc = _mm_set1_epi8((char)c);
		do {
			const __m128i xw = _mm_load_si128((const __m128i *)rp);
			__m128i xt = _mm_cmpeq_epi8(xw, xc);
			uint32_t mask = (uint32_t)_mm_movemask_epi8(xt);
			if(_MCFCRT_EXPECT_NOT(mask != 0)){
				return (char *)rp + __builtin_ctz(mask);
			}
			rp += 16;
		} while((size_t)(rend - rp) >= 16);
	}
	for(;;){
		if(rp == rend){
			return nullptr;
		}
		const char rc = *rp;
		if(rc == (char)c){
			return (char *)rp;
		}
		++rp;
	}
}
