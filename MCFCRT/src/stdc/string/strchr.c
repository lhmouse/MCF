// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../env/expect.h"
#include <pmmintrin.h>

#undef strchr

char *strchr(const char *s, int c){
	register const char *rp = s;
	// 如果 rp 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	while(((uintptr_t)rp & 31) != 0){
#define CHR_GEN()	\
		{	\
			const char rc = *rp;	\
			if(rc == (char)c){	\
				return (char *)rp;	\
			}	\
			if(rc == 0){	\
				return _MCFCRT_NULLPTR;	\
			}	\
			++rp;	\
		}
		CHR_GEN()
	}
#define CHR_SSE3()	\
	{	\
		const __m128i xc = _mm_set1_epi8((char)c);	\
		const __m128i xz = _mm_setzero_si128();	\
		for(;;){	\
			const __m128i xw0 = _mm_load_si128((const __m128i *)rp + 0);	\
			const __m128i xw1 = _mm_load_si128((const __m128i *)rp + 1);	\
			__m128i xt = _mm_cmpeq_epi8(xw0, xc);	\
			uint32_t mask = (uint32_t)_mm_movemask_epi8(xt);	\
			xt = _mm_cmpeq_epi8(xw1, xc);	\
			mask += (uint32_t)_mm_movemask_epi8(xt) << 16;	\
			if(_MCFCRT_EXPECT_NOT(mask != 0)){	\
				return (char *)rp + __builtin_ctzl(mask);	\
			}	\
			xt = _mm_cmpeq_epi8(xw0, xz);	\
			mask = (uint32_t)_mm_movemask_epi8(xt);	\
			xt = _mm_cmpeq_epi8(xw1, xz);	\
			mask += (uint32_t)_mm_movemask_epi8(xt) << 16;	\
			if(_MCFCRT_EXPECT_NOT(mask != 0)){	\
				return _MCFCRT_NULLPTR;	\
			}	\
			rp += 32;	\
		}	\
	}
	CHR_SSE3()
}
