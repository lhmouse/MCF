// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "wcpcpy.h"
#include "../env/expect.h"
#include "../env/crtassert.h"
#include "rep_movs.h"
#include <emmintrin.h>

wchar_t *_MCFCRT_wcpcpy(wchar_t *restrict dst, const wchar_t *restrict src){
	register wchar_t *wp = dst;
	register const wchar_t *rp = src;
	// 如果 rp 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	while(((uintptr_t)rp & 31) != 0){
#define CPY_GEN()	\
		{	\
			const wchar_t rc = *rp;	\
			*wp = rc;	\
			if(rc == 0){	\
				return wp;	\
			}	\
			++rp;	\
			++wp;	\
		}
		CPY_GEN()
	}
#define SSE2_CPY(save_, load_)	\
	{	\
		const __m128i xz = _mm_setzero_si128();	\
		for(;;){	\
			const __m128i xw0 = (load_)((const __m128i *)rp);	\
			const __m128i xw1 = (load_)((const __m128i *)rp + 1);	\
			__m128i xt = _mm_packs_epi16(_mm_cmpeq_epi16(xw0, xz), _mm_cmpeq_epi16(xw1, xz));	\
			uint32_t mask = (uint32_t)_mm_movemask_epi8(xt);	\
			if(_MCFCRT_EXPECT_NOT(mask != 0)){	\
				const unsigned tz = (unsigned)__builtin_ctz(mask);	\
				_MCFCRT_rep_movsw(wp, rp, tz);	\
				wp += tz;	\
				*wp = 0;	\
				return wp;	\
			}	\
			(save_)((__m128i *)wp, xw0);	\
			(save_)((__m128i *)wp + 1, xw1);	\
			rp += 16;	\
			wp += 16;	\
		}	\
	}
	if(((uintptr_t)wp & 15) == 0){
		SSE2_CPY(_mm_store_si128, _mm_load_si128)
	} else {
		SSE2_CPY(_mm_storeu_si128, _mm_load_si128)
	}
}
wchar_t *_MCFCRT_wcppcpy(wchar_t *dst, wchar_t *end, const wchar_t *restrict src){
	_MCFCRT_ASSERT(dst + 1 <= end);
	register wchar_t *wp = dst;
	wchar_t *const wend = end - 1;
	register const wchar_t *rp = src;
	// 如果 rp 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	while(((uintptr_t)rp & 31) != 0){
#define PCPY_GEN()	\
		{	\
			if(wp == wend){	\
				*wp = 0;	\
				return wp;	\
			}	\
			const wchar_t rc = *rp;	\
			*wp = rc;	\
			if(rc == 0){	\
				return wp;	\
			}	\
			++rp;	\
			++wp;	\
		}
		PCPY_GEN()
	}
	if((size_t)(wend - wp) >= 64){
#define SSE2_PCPY(save_, load_)	\
		{	\
			const __m128i xz = _mm_setzero_si128();	\
			do {	\
				const __m128i xw0 = (load_)((const __m128i *)rp);	\
				const __m128i xw1 = (load_)((const __m128i *)rp + 1);	\
				__m128i xt = _mm_packs_epi16(_mm_cmpeq_epi16(xw0, xz), _mm_cmpeq_epi16(xw1, xz));	\
				uint32_t mask = (uint32_t)_mm_movemask_epi8(xt);	\
				if(_MCFCRT_EXPECT_NOT(mask != 0)){	\
					const unsigned tz = (unsigned)__builtin_ctz(mask);	\
					_MCFCRT_rep_movsw(wp, rp, tz);	\
					wp += tz;	\
					*wp = 0;	\
					return wp;	\
				}	\
				(save_)((__m128i *)wp, xw0);	\
				(save_)((__m128i *)wp + 1, xw1);	\
				rp += 16;	\
				wp += 16;	\
			} while((size_t)(wend - wp) >= 16);	\
		}
		if(((uintptr_t)wp & 15) == 0){
			SSE2_PCPY(_mm_store_si128, _mm_load_si128)
		} else {
			SSE2_PCPY(_mm_storeu_si128, _mm_load_si128)
		}
	}
	for(;;){
		PCPY_GEN()
	}
}
