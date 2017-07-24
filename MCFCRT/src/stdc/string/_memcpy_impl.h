// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_STDC_STRING_MEMCPY_IMPL_H_
#define __MCFCRT_STDC_STRING_MEMCPY_IMPL_H_

#include "../../env/_crtdef.h"
#include "../../env/expect.h"
#include "../../ext/rep_movs.h"
#include <pmmintrin.h>

_MCFCRT_EXTERN_C_BEGIN

__attribute__((__always_inline__))
static inline void __MCFCRT_memcpy_impl_fwd(void *__s1, const void *__s2, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	register unsigned char *__wp = (unsigned char *)__s1;
	unsigned char *const __wend = __wp + __n;
	register const unsigned char *__rp = (const unsigned char *)__s2;
	if(_MCFCRT_EXPECT_NOT((size_t)(__wend - __wp) >= 256)){
		while(((_MCFCRT_STD uintptr_t)__wp & 15) != 0){
			if(__wp == __wend){
				return;
			}
			*(volatile unsigned char *)(__wp++) = *(__rp++);
		}
		_MCFCRT_STD size_t __t;
		if((__t = (_MCFCRT_STD size_t)(__wend - __wp) / 16) != 0){
#define __MCFCRT_SSE3_STEP_(__store_, __load_)	\
			{	\
				__store_((__m128i *)__wp, __load_((const __m128i *)__rp));	\
				__wp += 16;	\
				__rp += 16;	\
			}
#define __MCFCRT_SSE3_FULL_(__store_, __load_)	\
			{	\
				switch(__t % 8){	\
					do {	\
						_mm_prefetch(__rp + 256, _MM_HINT_NTA);	\
						_mm_prefetch(__rp + 320, _MM_HINT_NTA);	\
				__attribute__((__fallthrough__)); default: __MCFCRT_SSE3_STEP_(__store_, __load_)	\
				__attribute__((__fallthrough__)); case 7:  __MCFCRT_SSE3_STEP_(__store_, __load_)	\
				__attribute__((__fallthrough__)); case 6:  __MCFCRT_SSE3_STEP_(__store_, __load_)	\
				__attribute__((__fallthrough__)); case 5:  __MCFCRT_SSE3_STEP_(__store_, __load_)	\
				__attribute__((__fallthrough__)); case 4:  __MCFCRT_SSE3_STEP_(__store_, __load_)	\
				__attribute__((__fallthrough__)); case 3:  __MCFCRT_SSE3_STEP_(__store_, __load_)	\
				__attribute__((__fallthrough__)); case 2:  __MCFCRT_SSE3_STEP_(__store_, __load_)	\
				__attribute__((__fallthrough__)); case 1:  __MCFCRT_SSE3_STEP_(__store_, __load_)	\
					} while((size_t)(__wend - __wp) >= 128);	\
				}	\
			}
			if(_MCFCRT_EXPECT(__t < 0x1000)){
				if(((_MCFCRT_STD uintptr_t)__rp & 15) == 0){
					__MCFCRT_SSE3_FULL_(_mm_store_si128, _mm_load_si128)
				} else {
					__MCFCRT_SSE3_FULL_(_mm_store_si128, _mm_lddqu_si128)
				}
			} else {
				if(((_MCFCRT_STD uintptr_t)__rp & 15) == 0){
					__MCFCRT_SSE3_FULL_(_mm_stream_si128, _mm_load_si128)
				} else {
					__MCFCRT_SSE3_FULL_(_mm_stream_si128, _mm_lddqu_si128)
				}
				_mm_sfence();
			}
#undef __MCFCRT_SSE3_STEP_
#undef __MCFCRT_SSE3_FULL_
		}
	}
	_MCFCRT_rep_movsb(__wp, __rp, (size_t)(__wend - __wp));
}
__attribute__((__always_inline__))
static inline void __MCFCRT_memcpy_impl_bkwd(void *__s1, const void *__s2, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	unsigned char *const __wbegin = (unsigned char *)__s1;
	register unsigned char *__wp = __wbegin + __n;
	register const unsigned char *__rp = (const unsigned char *)__s2 + __n;
	if(_MCFCRT_EXPECT_NOT((size_t)(__wp - __wbegin) >= 256)){
		while(((_MCFCRT_STD uintptr_t)__wp & 15) != 0){
			if(__wbegin == __wp){
				return;
			}
			*(volatile unsigned char *)(--__wp) = *(--__rp);
		}
		_MCFCRT_STD size_t __t;
		if((__t = (_MCFCRT_STD size_t)(__wp - __wbegin) / 16) != 0){
#define __MCFCRT_SSE3_STEP_(__store_, __load_)	\
			{	\
				__wp -= 16;	\
				__rp -= 16;	\
				__store_((__m128i *)__wp, __load_((const __m128i *)__rp));	\
			}
#define __MCFCRT_SSE3_FULL_(__store_, __load_)	\
			{	\
				switch(__t % 8){	\
					do {	\
						_mm_prefetch(__rp - 384, _MM_HINT_NTA);	\
						_mm_prefetch(__rp - 320, _MM_HINT_NTA);	\
				__attribute__((__fallthrough__)); default: __MCFCRT_SSE3_STEP_(__store_, __load_)	\
				__attribute__((__fallthrough__)); case 7:  __MCFCRT_SSE3_STEP_(__store_, __load_)	\
				__attribute__((__fallthrough__)); case 6:  __MCFCRT_SSE3_STEP_(__store_, __load_)	\
				__attribute__((__fallthrough__)); case 5:  __MCFCRT_SSE3_STEP_(__store_, __load_)	\
				__attribute__((__fallthrough__)); case 4:  __MCFCRT_SSE3_STEP_(__store_, __load_)	\
				__attribute__((__fallthrough__)); case 3:  __MCFCRT_SSE3_STEP_(__store_, __load_)	\
				__attribute__((__fallthrough__)); case 2:  __MCFCRT_SSE3_STEP_(__store_, __load_)	\
				__attribute__((__fallthrough__)); case 1:  __MCFCRT_SSE3_STEP_(__store_, __load_)	\
					} while((size_t)(__wp - __wbegin) >= 128);	\
				}	\
			}
			if(_MCFCRT_EXPECT(__t < 0x1000)){
				if(((_MCFCRT_STD uintptr_t)__rp & 15) == 0){
					__MCFCRT_SSE3_FULL_(_mm_store_si128, _mm_load_si128)
				} else {
					__MCFCRT_SSE3_FULL_(_mm_store_si128, _mm_lddqu_si128)
				}
			} else {
				if(((_MCFCRT_STD uintptr_t)__rp & 15) == 0){
					__MCFCRT_SSE3_FULL_(_mm_stream_si128, _mm_load_si128)
				} else {
					__MCFCRT_SSE3_FULL_(_mm_stream_si128, _mm_lddqu_si128)
				}
				_mm_sfence();
			}
#undef __MCFCRT_SSE3_STEP_
#undef __MCFCRT_SSE3_FULL_
		}
	}
	_MCFCRT_STD size_t __t;
	for(__t = (_MCFCRT_STD size_t)(__wp - __wbegin) / sizeof(_MCFCRT_STD uintptr_t); __t != 0; --__t){
		__wp -= sizeof(_MCFCRT_STD uintptr_t);
		__rp -= sizeof(_MCFCRT_STD uintptr_t);
		*(volatile _MCFCRT_STD uintptr_t *)__wp = *(const _MCFCRT_STD uintptr_t *)__rp;
	}
	for(__t = (_MCFCRT_STD size_t)(__wp - __wbegin); __t != 0; --__t){
		__wp -= 1;
		__rp -= 1;
		*(volatile unsigned char *)__wp = *(const unsigned char *)__rp;
	}
}

_MCFCRT_EXTERN_C_END

#endif
