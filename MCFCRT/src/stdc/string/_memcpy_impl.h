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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"

__attribute__((__always_inline__))
static inline void __MCFCRT_memcpy_impl_fwd(void *__s1, const void *__s2, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	register unsigned char *__wp __asm__("di") = (unsigned char *)__s1;
	register const unsigned char *__rp __asm__("si") = (const unsigned char *)__s2;
	_MCFCRT_STD size_t __total = __n;
	_MCFCRT_STD size_t __rem;
	__rem = __total / 16;
	if(_MCFCRT_EXPECT_NOT(__rem >= 4)){
		while(((_MCFCRT_STD uintptr_t)__wp & ~(_MCFCRT_STD uintptr_t)-16) != 0){
			*(volatile unsigned char *)(__wp++) = *(__rp++);
			--__total;
		}
#define __MCFCRT_SSE3_STEP_(__store_, __load_)	\
		__store_((__m128i *)__wp, __load_((const __m128i *)__rp));	\
		__wp += 16;	\
		__rp += 16;	\
		--__rem;
#define __MCFCRT_SSE3_FULL_(__store_, __load_)	\
		switch(__rem % 8){	\
			do {	\
		default: __MCFCRT_SSE3_STEP_(__store_, __load_)	\
		case 7:  __MCFCRT_SSE3_STEP_(__store_, __load_)	\
		case 6:  __MCFCRT_SSE3_STEP_(__store_, __load_)	\
		case 5:  __MCFCRT_SSE3_STEP_(__store_, __load_)	\
		case 4:  __MCFCRT_SSE3_STEP_(__store_, __load_)	\
		case 3:  __MCFCRT_SSE3_STEP_(__store_, __load_)	\
		case 2:  __MCFCRT_SSE3_STEP_(__store_, __load_)	\
		case 1:  __MCFCRT_SSE3_STEP_(__store_, __load_)	\
			} while(_MCFCRT_EXPECT(__rem != 0));	\
		}
//=============================================================================
		if(_MCFCRT_EXPECT(__total < 0x4000)){
			if(((_MCFCRT_STD uintptr_t)__rp & ~(_MCFCRT_STD uintptr_t)-16) == 0){
				__MCFCRT_SSE3_FULL_(_mm_store_si128, _mm_load_si128)
			} else {
				__MCFCRT_SSE3_FULL_(_mm_store_si128, _mm_lddqu_si128)
			}
		} else {
			if(((_MCFCRT_STD uintptr_t)__rp & ~(_MCFCRT_STD uintptr_t)-16) == 0){
				__MCFCRT_SSE3_FULL_(_mm_stream_si128, _mm_load_si128)
			} else {
				__MCFCRT_SSE3_FULL_(_mm_stream_si128, _mm_lddqu_si128)
			}
			_mm_sfence();
		}
//=============================================================================
#undef __MCFCRT_SSE3_STEP_
#undef __MCFCRT_SSE3_FULL_
	}
	__rem = __rem * 16 + __total % 16;
	_MCFCRT_rep_movsb(_MCFCRT_NULLPTR, __wp, __rp, __rem);
}

__attribute__((__always_inline__))
static inline void __MCFCRT_memcpy_impl_bkwd(void *__s1, const void *__s2, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	register unsigned char *__wp __asm__("di") = (unsigned char *)__s1;
	register const unsigned char *__rp __asm__("si") = (const unsigned char *)__s2;
	_MCFCRT_STD size_t __total = __n;
	_MCFCRT_STD size_t __rem;
	__rem = __total / 16;
	if(_MCFCRT_EXPECT_NOT(__rem >= 4)){
		while(((_MCFCRT_STD uintptr_t)__wp & ~(_MCFCRT_STD uintptr_t)-16) != 0){
			*(volatile unsigned char *)(--__wp) = *(--__rp);
			--__total;
		}
#define __MCFCRT_SSE3_STEP_(__store_, __load_)	\
		__wp -= 16;	\
		__rp -= 16;	\
		__store_((__m128i *)__wp, __load_((const __m128i *)__rp));	\
		--__rem;
#define __MCFCRT_SSE3_FULL_(__store_, __load_)	\
		switch(__rem % 8){	\
			do {	\
		default: __MCFCRT_SSE3_STEP_(__store_, __load_)	\
		case 7:  __MCFCRT_SSE3_STEP_(__store_, __load_)	\
		case 6:  __MCFCRT_SSE3_STEP_(__store_, __load_)	\
		case 5:  __MCFCRT_SSE3_STEP_(__store_, __load_)	\
		case 4:  __MCFCRT_SSE3_STEP_(__store_, __load_)	\
		case 3:  __MCFCRT_SSE3_STEP_(__store_, __load_)	\
		case 2:  __MCFCRT_SSE3_STEP_(__store_, __load_)	\
		case 1:  __MCFCRT_SSE3_STEP_(__store_, __load_)	\
			} while(_MCFCRT_EXPECT(__rem != 0));	\
		}
//=============================================================================
		if(_MCFCRT_EXPECT(__total < 0x4000)){
			if(((_MCFCRT_STD uintptr_t)__rp & ~(_MCFCRT_STD uintptr_t)-16) == 0){
				__MCFCRT_SSE3_FULL_(_mm_store_si128, _mm_load_si128)
			} else {
				__MCFCRT_SSE3_FULL_(_mm_store_si128, _mm_lddqu_si128)
			}
		} else {
			if(((_MCFCRT_STD uintptr_t)__rp & ~(_MCFCRT_STD uintptr_t)-16) == 0){
				__MCFCRT_SSE3_FULL_(_mm_stream_si128, _mm_load_si128)
			} else {
				__MCFCRT_SSE3_FULL_(_mm_stream_si128, _mm_lddqu_si128)
			}
			_mm_sfence();
		}
//=============================================================================
#undef __MCFCRT_SSE3_STEP_
#undef __MCFCRT_SSE3_FULL_
	}
	__rem = __rem * 16 + __total % 16;
	while(__rem != 0){
		*(volatile unsigned char *)(--__wp) = *(--__rp);
		--__rem;
	}
}

#pragma GCC diagnostic pop

_MCFCRT_EXTERN_C_END

#endif
