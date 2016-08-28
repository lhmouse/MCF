// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../ext/expect.h"
#include <emmintrin.h>

size_t strlen(const char *s){
	register const char *rp = s;
	// 如果 rp 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	while(((uintptr_t)rp & 15) != 0){
		const char rc = *rp;
		if(rc == 0){
			return (size_t)(rp - s);
		}
		++rp;
	}
	const __m128i xz = _mm_setzero_si128();
	for(;;){
		const __m128i xw = _mm_load_si128((const __m128i *)rp);
		__m128i xt = _mm_cmpeq_epi8(xw, xz);
		unsigned mask = (unsigned)_mm_movemask_epi8(xt);
		if(_MCFCRT_EXPECT_NOT(mask != 0)){
			return (size_t)(rp - s + __builtin_ctz(mask));
		}
		rp += 16;
	}
}
