// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_STRING_MEMCPY_INL_H_
#define __MCFCRT_STRING_MEMCPY_INL_H_

#include "../../env/_crtdef.h"
#include "../../ext/expect.h"
#include <intrin.h>
#include <emmintrin.h>

_MCFCRT_EXTERN_C_BEGIN

__attribute__((__always_inline__))
static inline void __MCFCRT_CopyForward(void *__s1, const void *__s2, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	register char *__wp = __s1;
	char *const __wend = __wp + __n;
	register const char *__rp = __s2;
	if((size_t)(__wend - __wp) >= 256){
		while(((_MCFCRT_STD uintptr_t)__wp & 15) != 0){
			if(__wp == __wend){
				return;
			}
			*(volatile char *)(__wp++) = *(__rp++);
		}
		_MCFCRT_STD size_t __t;
		if((__t = (_MCFCRT_STD size_t)(__wend - __wp) / 16) != 0){
#define __MCFCRT_SSE2_STEP(__si_, __li_)	\
			{	\
				(__si_)((__m128i *)__wp, (__li_)((const __m128i *)__rp));	\
				__wp += 16;	\
				__rp += 16;	\
			}
#define __MCFCRT_SSE2_FULL(__si_, __li_)	\
			{	\
				switch(__t % 8){	\
					do {	\
						_mm_prefetch(__rp + 256, _MM_HINT_NTA);	\
						_mm_prefetch(__rp + 320, _MM_HINT_NTA);	\
				default: __MCFCRT_SSE2_STEP(__si_, __li_)	\
				case 7:  __MCFCRT_SSE2_STEP(__si_, __li_)	\
				case 6:  __MCFCRT_SSE2_STEP(__si_, __li_)	\
				case 5:  __MCFCRT_SSE2_STEP(__si_, __li_)	\
				case 4:  __MCFCRT_SSE2_STEP(__si_, __li_)	\
				case 3:  __MCFCRT_SSE2_STEP(__si_, __li_)	\
				case 2:  __MCFCRT_SSE2_STEP(__si_, __li_)	\
				case 1:  __MCFCRT_SSE2_STEP(__si_, __li_)	\
					} while((size_t)(__wend - __wp) >= 128);	\
				}	\
			}
			if(_MCFCRT_EXPECT(__t < 0x1000)){
				if(((_MCFCRT_STD uintptr_t)__rp & 15) == 0){
					__MCFCRT_SSE2_FULL(_mm_store_si128, _mm_load_si128)
				} else {
					__MCFCRT_SSE2_FULL(_mm_store_si128, _mm_loadu_si128)
				}
			} else {
				if(((_MCFCRT_STD uintptr_t)__rp & 15) == 0){
					__MCFCRT_SSE2_FULL(_mm_stream_si128, _mm_load_si128)
				} else {
					__MCFCRT_SSE2_FULL(_mm_stream_si128, _mm_loadu_si128)
				}
				_mm_sfence();
			}
#undef __MCFCRT_SSE2_STEP
#undef __MCFCRT_SSE2_FULL
		}
	}
	__movsb((unsigned char *)__wp, (const unsigned char *)__rp, (size_t)(__wend - __wp));
}
__attribute__((__always_inline__))
static inline void __MCFCRT_CopyBackward(void *__s1, const void *__s2, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	char *const __wbegin = __s1;
	register char *__wp = __wbegin + __n;
	register const char *__rp = (const char *)__s2 + __n;
	if((size_t)(__wp - __wbegin) >= 256){
		while(((_MCFCRT_STD uintptr_t)__wp & 15) != 0){
			if(__wbegin == __wp){
				return;
			}
			*(volatile char *)(--__wp) = *(--__rp);
		}
		_MCFCRT_STD size_t __t;
		if((__t = (_MCFCRT_STD size_t)(__wp - __wbegin) / 16) != 0){
#define __MCFCRT_SSE2_STEP(__si_, __li_)	\
			{	\
				__wp -= 16;	\
				__rp -= 16;	\
				(__si_)((__m128i *)__wp, (__li_)((const __m128i *)__rp));	\
			}
#define __MCFCRT_SSE2_FULL(__si_, __li_)	\
			{	\
				switch(__t % 8){	\
					do {	\
						_mm_prefetch(__rp - 384, _MM_HINT_NTA);	\
						_mm_prefetch(__rp - 320, _MM_HINT_NTA);	\
				default: __MCFCRT_SSE2_STEP(__si_, __li_)	\
				case 7:  __MCFCRT_SSE2_STEP(__si_, __li_)	\
				case 6:  __MCFCRT_SSE2_STEP(__si_, __li_)	\
				case 5:  __MCFCRT_SSE2_STEP(__si_, __li_)	\
				case 4:  __MCFCRT_SSE2_STEP(__si_, __li_)	\
				case 3:  __MCFCRT_SSE2_STEP(__si_, __li_)	\
				case 2:  __MCFCRT_SSE2_STEP(__si_, __li_)	\
				case 1:  __MCFCRT_SSE2_STEP(__si_, __li_)	\
					} while((size_t)(__wp - __wbegin) >= 128);	\
				}	\
			}
			if(_MCFCRT_EXPECT(__t < 0x1000)){
				if(((_MCFCRT_STD uintptr_t)__rp & 15) == 0){
					__MCFCRT_SSE2_FULL(_mm_store_si128, _mm_load_si128)
				} else {
					__MCFCRT_SSE2_FULL(_mm_store_si128, _mm_loadu_si128)
				}
			} else {
				if(((_MCFCRT_STD uintptr_t)__rp & 15) == 0){
					__MCFCRT_SSE2_FULL(_mm_stream_si128, _mm_load_si128)
				} else {
					__MCFCRT_SSE2_FULL(_mm_stream_si128, _mm_loadu_si128)
				}
				_mm_sfence();
			}
#undef __MCFCRT_SSE2_STEP
#undef __MCFCRT_SSE2_FULL
		}
	}
	while(__wbegin != __wp){
		*(volatile char *)(--__wp) = *(--__rp);
	}
}

_MCFCRT_EXTERN_C_END

#endif
