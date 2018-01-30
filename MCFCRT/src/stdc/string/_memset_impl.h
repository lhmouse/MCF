// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_STDC_STRING_MEMSET_IMPL_H_
#define __MCFCRT_STDC_STRING_MEMSET_IMPL_H_

#include "../../env/_crtdef.h"
#include "../../env/expect.h"
#include "../../env/xassert.h"
#include <emmintrin.h>

#ifndef __MCFCRT_MEMSET_IMPL_INLINE_OR_EXTERN
#  define __MCFCRT_MEMSET_IMPL_INLINE_OR_EXTERN     __attribute__((__gnu_inline__)) extern inline
#endif

_MCFCRT_EXTERN_C_BEGIN

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-unreachable"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#pragma GCC diagnostic ignored "-Wconversion"

__attribute__((__gnu_inline__, __always_inline__))
extern inline void __MCFCRT_memset_piece01_fwd(unsigned char **_MCFCRT_RESTRICT __wp, _MCFCRT_STD uint8_t __c) _MCFCRT_NOEXCEPT {
	*(volatile _MCFCRT_STD uint8_t *)*__wp = __c;
	*__wp += 1;
}
__attribute__((__gnu_inline__, __always_inline__))
extern inline void __MCFCRT_memset_piece01_bwd(unsigned char **_MCFCRT_RESTRICT __wp, _MCFCRT_STD uint8_t __c) _MCFCRT_NOEXCEPT {
	*__wp -= 1;
	*(volatile _MCFCRT_STD uint8_t *)*__wp = __c;
}

__attribute__((__gnu_inline__, __always_inline__))
extern inline void __MCFCRT_memset_piece02_fwd(unsigned char **_MCFCRT_RESTRICT __wp, _MCFCRT_STD uint16_t __c) _MCFCRT_NOEXCEPT {
	*(volatile _MCFCRT_STD uint16_t *)*__wp = __c;
	*__wp += 2;
}
__attribute__((__gnu_inline__, __always_inline__))
extern inline void __MCFCRT_memset_piece02_bwd(unsigned char **_MCFCRT_RESTRICT __wp, _MCFCRT_STD uint16_t __c) _MCFCRT_NOEXCEPT {
	*__wp -= 2;
	*(volatile _MCFCRT_STD uint16_t *)*__wp = __c;
}

__attribute__((__gnu_inline__, __always_inline__))
extern inline void __MCFCRT_memset_piece04_fwd(unsigned char **_MCFCRT_RESTRICT __wp, _MCFCRT_STD uint32_t __c) _MCFCRT_NOEXCEPT {
	*(volatile _MCFCRT_STD uint32_t *)*__wp = __c;
	*__wp += 4;
}
__attribute__((__gnu_inline__, __always_inline__))
extern inline void __MCFCRT_memset_piece04_bwd(unsigned char **_MCFCRT_RESTRICT __wp, _MCFCRT_STD uint32_t __c) _MCFCRT_NOEXCEPT {
	*__wp -= 4;
	*(volatile _MCFCRT_STD uint32_t *)*__wp = __c;
}

__attribute__((__gnu_inline__, __always_inline__))
extern inline void __MCFCRT_memset_piece08_fwd(unsigned char **_MCFCRT_RESTRICT __wp, __m128 __x) _MCFCRT_NOEXCEPT {
	_mm_storel_pi((__m64 *)*__wp, __x);
	*__wp += 8;
}
__attribute__((__gnu_inline__, __always_inline__))
extern inline void __MCFCRT_memset_piece08_bwd(unsigned char **_MCFCRT_RESTRICT __wp, __m128 __x) _MCFCRT_NOEXCEPT {
	*__wp -= 8;
	_mm_storel_pi((__m64 *)*__wp, __x);
}

__attribute__((__gnu_inline__, __always_inline__))
extern inline void __MCFCRT_memset_piece16_fwd(unsigned char **_MCFCRT_RESTRICT __wp, __m128 __x) _MCFCRT_NOEXCEPT {
	_mm_storeu_ps((float *)*__wp, __x);
	*__wp += 16;
}
__attribute__((__gnu_inline__, __always_inline__))
extern inline void __MCFCRT_memset_piece16_bwd(unsigned char **_MCFCRT_RESTRICT __wp, __m128 __x) _MCFCRT_NOEXCEPT {
	*__wp -= 16;
	_mm_storeu_ps((float *)*__wp, __x);
}

__attribute__((__gnu_inline__, __always_inline__))
extern inline void __MCFCRT_memset_piece32_fwd(unsigned char **_MCFCRT_RESTRICT __wp, __m128 __x) _MCFCRT_NOEXCEPT {
	// TODO: Rewrite to make use of AVX in the future.
	_mm_storeu_ps((float *)*__wp, __x);
	*__wp += 16;
	_mm_storeu_ps((float *)*__wp, __x);
	*__wp += 16;
}
__attribute__((__gnu_inline__, __always_inline__))
extern inline void __MCFCRT_memset_piece32_bwd(unsigned char **_MCFCRT_RESTRICT __wp, __m128 __x) _MCFCRT_NOEXCEPT {
	// TODO: Rewrite to make use of AVX in the future.
	*__wp -= 16;
	_mm_storeu_ps((float *)*__wp, __x);
	*__wp -= 16;
	_mm_storeu_ps((float *)*__wp, __x);
}

__attribute__((__gnu_inline__, __always_inline__))
extern inline void __MCFCRT_memset_aligned32_fwd(unsigned char **_MCFCRT_RESTRICT __wp, __m128 __x) _MCFCRT_NOEXCEPT {
	// TODO: Rewrite to make use of AVX in the future.
	_mm_store_ps((float *)*__wp, __x);
	*__wp += 16;
	_mm_store_ps((float *)*__wp, __x);
	*__wp += 16;
}
__attribute__((__gnu_inline__, __always_inline__))
extern inline void __MCFCRT_memset_aligned32_bwd(unsigned char **_MCFCRT_RESTRICT __wp, __m128 __x) _MCFCRT_NOEXCEPT {
	// TODO: Rewrite to make use of AVX in the future.
	*__wp -= 16;
	_mm_store_ps((float *)*__wp, __x);
	*__wp -= 16;
	_mm_store_ps((float *)*__wp, __x);
}

__attribute__((__gnu_inline__, __always_inline__))
extern inline void __MCFCRT_memset_nontemp32_fwd(unsigned char **_MCFCRT_RESTRICT __wp, __m128 __x) _MCFCRT_NOEXCEPT {
	// TODO: Rewrite to make use of AVX in the future.
	_mm_stream_ps((float *)*__wp, __x);
	*__wp += 16;
	_mm_stream_ps((float *)*__wp, __x);
	*__wp += 16;
}
__attribute__((__gnu_inline__, __always_inline__))
extern inline void __MCFCRT_memset_nontemp32_bwd(unsigned char **_MCFCRT_RESTRICT __wp, __m128 __x) _MCFCRT_NOEXCEPT {
	// TODO: Rewrite to make use of AVX in the future.
	*__wp -= 16;
	_mm_stream_ps((float *)*__wp, __x);
	*__wp -= 16;
	_mm_stream_ps((float *)*__wp, __x);
}

// Functions that fill blocks no smaller than 64 bytes.
__MCFCRT_MEMSET_IMPL_INLINE_OR_EXTERN void __MCFCRT_memset_large_fwd(unsigned char *__bwp, unsigned char *__ewp, __m128 __x){
	_MCFCRT_ASSERT(__ewp - __bwp >= 64);
	// Fill the initial, potentially unaligned QQWORD.
	unsigned char *__wp = __bwp;
	__MCFCRT_memset_piece32_fwd(&__wp, __x);
	// If there is misalignment at all, align the write pointer to 32-byte boundaries, rounding downwards.
	// We will fill the final QQWORD separately, hence the last (0,32] bytes are excluded here.
	__wp = (unsigned char *)((_MCFCRT_STD uintptr_t)(__bwp + 32) & (_MCFCRT_STD uintptr_t)-32);
	// Fill QQWORDs in aligned locations.
	switch((_MCFCRT_STD size_t)(__ewp - __wp - 1) / 32 % 16){
		do {
#define __MCFCRT_FILL_STEP_(k_)	\
		__MCFCRT_memset_aligned32_fwd(&__wp, __x);	\
	case (k_):	\
		;
//=============================================================================
	__MCFCRT_FILL_STEP_(017)  __MCFCRT_FILL_STEP_(016)  __MCFCRT_FILL_STEP_(015)  __MCFCRT_FILL_STEP_(014)
	__MCFCRT_FILL_STEP_(013)  __MCFCRT_FILL_STEP_(012)  __MCFCRT_FILL_STEP_(011)  __MCFCRT_FILL_STEP_(010)
	__MCFCRT_FILL_STEP_(007)  __MCFCRT_FILL_STEP_(006)  __MCFCRT_FILL_STEP_(005)  __MCFCRT_FILL_STEP_(004)
	__MCFCRT_FILL_STEP_(003)  __MCFCRT_FILL_STEP_(002)  __MCFCRT_FILL_STEP_(001)  __MCFCRT_FILL_STEP_(000)
//=============================================================================
#undef __MCFCRT_FILL_STEP_
		} while(_MCFCRT_EXPECT((_MCFCRT_STD size_t)(__ewp - __wp) > 32));
	}
	// Fill the final, potentially unaligned QQWORD.
	__wp = __ewp;
	__MCFCRT_memset_piece32_bwd(&__wp, __x);
}
__MCFCRT_MEMSET_IMPL_INLINE_OR_EXTERN void __MCFCRT_memset_large_bwd(unsigned char *__bwp, unsigned char *__ewp, __m128 __x){
	_MCFCRT_ASSERT(__ewp - __bwp >= 64);
	// Fill the final, potentially unaligned QQWORD.
	unsigned char *__wp = __ewp;
	__MCFCRT_memset_piece32_bwd(&__wp, __x);
	// If there is misalignment at all, align the write pointer to 32-byte boundaries, rounding downwards.
	// We will fill the initial QQWORD separately, hence the last (0,32] bytes are excluded here.
	__wp = (unsigned char *)((_MCFCRT_STD uintptr_t)(__ewp - 1) & (_MCFCRT_STD uintptr_t)-32);
	// Fill QQWORDs to aligned locations.
	switch((_MCFCRT_STD size_t)(__wp - __bwp - 1) / 32 % 16){
		do {
#define __MCFCRT_FILL_STEP_(k_)	\
		__MCFCRT_memset_aligned32_bwd(&__wp, __x);	\
	case (k_):	\
		;
//=============================================================================
	__MCFCRT_FILL_STEP_(017)  __MCFCRT_FILL_STEP_(016)  __MCFCRT_FILL_STEP_(015)  __MCFCRT_FILL_STEP_(014)
	__MCFCRT_FILL_STEP_(013)  __MCFCRT_FILL_STEP_(012)  __MCFCRT_FILL_STEP_(011)  __MCFCRT_FILL_STEP_(010)
	__MCFCRT_FILL_STEP_(007)  __MCFCRT_FILL_STEP_(006)  __MCFCRT_FILL_STEP_(005)  __MCFCRT_FILL_STEP_(004)
	__MCFCRT_FILL_STEP_(003)  __MCFCRT_FILL_STEP_(002)  __MCFCRT_FILL_STEP_(001)  __MCFCRT_FILL_STEP_(000)
//=============================================================================
#undef __MCFCRT_FILL_STEP_
		} while(_MCFCRT_EXPECT((_MCFCRT_STD size_t)(__wp - __bwp) > 32));
	}
	// Fill the initial, potentially unaligned QQWORD.
	__wp = __bwp;
	__MCFCRT_memset_piece32_fwd(&__wp, __x);
}

// Functions that fill blocks no smaller than 64 bytes using non-temporal semantics.
__MCFCRT_MEMSET_IMPL_INLINE_OR_EXTERN void __MCFCRT_memset_huge_fwd(unsigned char *__bwp, unsigned char *__ewp, __m128 __x){
	_MCFCRT_ASSERT(__ewp - __bwp >= 64);
	// Fill the initial, potentially unaligned QQWORD.
	unsigned char *__wp = __bwp;
	__MCFCRT_memset_piece32_fwd(&__wp, __x);
	// If there is misalignment at all, align the write pointer to 32-byte boundaries, rounding downwards.
	// We will fill the final QQWORD separately, hence the last (0,32] bytes are excluded here.
	__wp = (unsigned char *)((_MCFCRT_STD uintptr_t)(__bwp + 32) & (_MCFCRT_STD uintptr_t)-32);
	// Fill QQWORDs in aligned locations.
	switch((_MCFCRT_STD size_t)(__ewp - __wp - 1) / 32 % 16){
		do {
#define __MCFCRT_FILL_STEP_(k_)	\
		__MCFCRT_memset_nontemp32_fwd(&__wp, __x);	\
	case (k_):	\
		;
//=============================================================================
	__MCFCRT_FILL_STEP_(017)  __MCFCRT_FILL_STEP_(016)  __MCFCRT_FILL_STEP_(015)  __MCFCRT_FILL_STEP_(014)
	__MCFCRT_FILL_STEP_(013)  __MCFCRT_FILL_STEP_(012)  __MCFCRT_FILL_STEP_(011)  __MCFCRT_FILL_STEP_(010)
	__MCFCRT_FILL_STEP_(007)  __MCFCRT_FILL_STEP_(006)  __MCFCRT_FILL_STEP_(005)  __MCFCRT_FILL_STEP_(004)
	__MCFCRT_FILL_STEP_(003)  __MCFCRT_FILL_STEP_(002)  __MCFCRT_FILL_STEP_(001)  __MCFCRT_FILL_STEP_(000)
//=============================================================================
#undef __MCFCRT_FILL_STEP_
		} while(_MCFCRT_EXPECT((_MCFCRT_STD size_t)(__ewp - __wp) > 32));
	}
	// Don't forget the a store fence.
	_mm_sfence();
	// Fill the final, potentially unaligned QQWORD.
	__wp = __ewp;
	__MCFCRT_memset_piece32_bwd(&__wp, __x);
}
__MCFCRT_MEMSET_IMPL_INLINE_OR_EXTERN void __MCFCRT_memset_huge_bwd(unsigned char *__bwp, unsigned char *__ewp, __m128 __x){
	_MCFCRT_ASSERT(__ewp - __bwp >= 64);
	// Fill the final, potentially unaligned QQWORD.
	unsigned char *__wp = __ewp;
	__MCFCRT_memset_piece32_bwd(&__wp, __x);
	// If there is misalignment at all, align the write pointer to 32-byte boundaries, rounding downwards.
	// We will fill the initial QQWORD separately, hence the last (0,32] bytes are excluded here.
	__wp = (unsigned char *)((_MCFCRT_STD uintptr_t)(__ewp - 1) & (_MCFCRT_STD uintptr_t)-32);
	// Fill QQWORDs to aligned locations.
	switch((_MCFCRT_STD size_t)(__wp - __bwp - 1) / 32 % 16){
		do {
#define __MCFCRT_FILL_STEP_(k_)	\
		__MCFCRT_memset_nontemp32_bwd(&__wp, __x);	\
	case (k_):	\
		;
//=============================================================================
	__MCFCRT_FILL_STEP_(017)  __MCFCRT_FILL_STEP_(016)  __MCFCRT_FILL_STEP_(015)  __MCFCRT_FILL_STEP_(014)
	__MCFCRT_FILL_STEP_(013)  __MCFCRT_FILL_STEP_(012)  __MCFCRT_FILL_STEP_(011)  __MCFCRT_FILL_STEP_(010)
	__MCFCRT_FILL_STEP_(007)  __MCFCRT_FILL_STEP_(006)  __MCFCRT_FILL_STEP_(005)  __MCFCRT_FILL_STEP_(004)
	__MCFCRT_FILL_STEP_(003)  __MCFCRT_FILL_STEP_(002)  __MCFCRT_FILL_STEP_(001)  __MCFCRT_FILL_STEP_(000)
//=============================================================================
#undef __MCFCRT_FILL_STEP_
		} while(_MCFCRT_EXPECT((_MCFCRT_STD size_t)(__wp - __bwp) > 32));
	}
	// Don't forget the a store fence.
	_mm_sfence();
	// Fill the initial, potentially unaligned QQWORD.
	__wp = __bwp;
	__MCFCRT_memset_piece32_fwd(&__wp, __x);
}

// Dispatchers.
__MCFCRT_MEMSET_IMPL_INLINE_OR_EXTERN void __MCFCRT_memset_impl_fwd(unsigned char *__bwp, unsigned char *__ewp, _MCFCRT_STD uint32_t __c) _MCFCRT_NOEXCEPT {
	unsigned char *__wp = __bwp;
	__m128 __x = _mm_castsi128_ps(_mm_set1_epi32((int)__c));
	switch((_MCFCRT_STD size_t)(__ewp - __wp)){
	          // Deal with empty blocks.
	case  0:  break;
	          // Deal with tiny blocks using a detailed table.
	case 63:  __MCFCRT_memset_piece32_fwd(&__wp, __x);
	case 31:  __MCFCRT_memset_piece16_fwd(&__wp, __x);
	case 15:  __MCFCRT_memset_piece08_fwd(&__wp, __x);
	case  7:  __MCFCRT_memset_piece04_fwd(&__wp, __c);
	case  3:  __MCFCRT_memset_piece02_fwd(&__wp, __c);
	case  1:  __MCFCRT_memset_piece01_fwd(&__wp, __c);
	          break;
	case 62:  __MCFCRT_memset_piece32_fwd(&__wp, __x);
	case 30:  __MCFCRT_memset_piece16_fwd(&__wp, __x);
	case 14:  __MCFCRT_memset_piece08_fwd(&__wp, __x);
	case  6:  __MCFCRT_memset_piece04_fwd(&__wp, __c);
	case  2:  __MCFCRT_memset_piece02_fwd(&__wp, __c);
	          break;
	case 61:  __MCFCRT_memset_piece32_fwd(&__wp, __x);
	case 29:  __MCFCRT_memset_piece16_fwd(&__wp, __x);
	case 13:  __MCFCRT_memset_piece08_fwd(&__wp, __x);
	case  5:  __MCFCRT_memset_piece04_fwd(&__wp, __c);
	          __MCFCRT_memset_piece01_fwd(&__wp, __c);
	          break;
	case 60:  __MCFCRT_memset_piece32_fwd(&__wp, __x);
	case 28:  __MCFCRT_memset_piece16_fwd(&__wp, __x);
	case 12:  __MCFCRT_memset_piece08_fwd(&__wp, __x);
	case  4:  __MCFCRT_memset_piece04_fwd(&__wp, __c);
	          break;
	case 59:  __MCFCRT_memset_piece32_fwd(&__wp, __x);
	case 27:  __MCFCRT_memset_piece16_fwd(&__wp, __x);
	case 11:  __MCFCRT_memset_piece08_fwd(&__wp, __x);
	          __MCFCRT_memset_piece02_fwd(&__wp, __c);
	          __MCFCRT_memset_piece01_fwd(&__wp, __c);
	          break;
	case 58:  __MCFCRT_memset_piece32_fwd(&__wp, __x);
	case 26:  __MCFCRT_memset_piece16_fwd(&__wp, __x);
	case 10:  __MCFCRT_memset_piece08_fwd(&__wp, __x);
	          __MCFCRT_memset_piece02_fwd(&__wp, __c);
	          break;
	case 57:  __MCFCRT_memset_piece32_fwd(&__wp, __x);
	case 25:  __MCFCRT_memset_piece16_fwd(&__wp, __x);
	case  9:  __MCFCRT_memset_piece08_fwd(&__wp, __x);
	          __MCFCRT_memset_piece01_fwd(&__wp, __c);
	          break;
	case 56:  __MCFCRT_memset_piece32_fwd(&__wp, __x);
	case 24:  __MCFCRT_memset_piece16_fwd(&__wp, __x);
	case  8:  __MCFCRT_memset_piece08_fwd(&__wp, __x);
	          break;
	case 55:  __MCFCRT_memset_piece32_fwd(&__wp, __x);
	case 23:  __MCFCRT_memset_piece16_fwd(&__wp, __x);
	          __MCFCRT_memset_piece04_fwd(&__wp, __c);
	          __MCFCRT_memset_piece02_fwd(&__wp, __c);
	          __MCFCRT_memset_piece01_fwd(&__wp, __c);
	          break;
	case 54:  __MCFCRT_memset_piece32_fwd(&__wp, __x);
	case 22:  __MCFCRT_memset_piece16_fwd(&__wp, __x);
	          __MCFCRT_memset_piece04_fwd(&__wp, __c);
	          __MCFCRT_memset_piece02_fwd(&__wp, __c);
	          break;
	case 53:  __MCFCRT_memset_piece32_fwd(&__wp, __x);
	case 21:  __MCFCRT_memset_piece16_fwd(&__wp, __x);
	          __MCFCRT_memset_piece04_fwd(&__wp, __c);
	          __MCFCRT_memset_piece01_fwd(&__wp, __c);
	          break;
	case 52:  __MCFCRT_memset_piece32_fwd(&__wp, __x);
	case 20:  __MCFCRT_memset_piece16_fwd(&__wp, __x);
	          __MCFCRT_memset_piece04_fwd(&__wp, __c);
	          break;
	case 51:  __MCFCRT_memset_piece32_fwd(&__wp, __x);
	case 19:  __MCFCRT_memset_piece16_fwd(&__wp, __x);
	          __MCFCRT_memset_piece02_fwd(&__wp, __c);
	          __MCFCRT_memset_piece01_fwd(&__wp, __c);
	          break;
	case 50:  __MCFCRT_memset_piece32_fwd(&__wp, __x);
	case 18:  __MCFCRT_memset_piece16_fwd(&__wp, __x);
	          __MCFCRT_memset_piece02_fwd(&__wp, __c);
	          break;
	case 49:  __MCFCRT_memset_piece32_fwd(&__wp, __x);
	case 17:  __MCFCRT_memset_piece16_fwd(&__wp, __x);
	          __MCFCRT_memset_piece01_fwd(&__wp, __c);
	          break;
	case 48:  __MCFCRT_memset_piece32_fwd(&__wp, __x);
	case 16:  __MCFCRT_memset_piece16_fwd(&__wp, __x);
	          break;
	case 47:  __MCFCRT_memset_piece32_fwd(&__wp, __x);
	          __MCFCRT_memset_piece08_fwd(&__wp, __x);
	          __MCFCRT_memset_piece04_fwd(&__wp, __c);
	          __MCFCRT_memset_piece02_fwd(&__wp, __c);
	          __MCFCRT_memset_piece01_fwd(&__wp, __c);
	          break;
	case 46:  __MCFCRT_memset_piece32_fwd(&__wp, __x);
	          __MCFCRT_memset_piece08_fwd(&__wp, __x);
	          __MCFCRT_memset_piece04_fwd(&__wp, __c);
	          __MCFCRT_memset_piece02_fwd(&__wp, __c);
	          break;
	case 45:  __MCFCRT_memset_piece32_fwd(&__wp, __x);
	          __MCFCRT_memset_piece08_fwd(&__wp, __x);
	          __MCFCRT_memset_piece04_fwd(&__wp, __c);
	          __MCFCRT_memset_piece01_fwd(&__wp, __c);
	          break;
	case 44:  __MCFCRT_memset_piece32_fwd(&__wp, __x);
	          __MCFCRT_memset_piece08_fwd(&__wp, __x);
	          __MCFCRT_memset_piece04_fwd(&__wp, __c);
	          break;
	case 43:  __MCFCRT_memset_piece32_fwd(&__wp, __x);
	          __MCFCRT_memset_piece08_fwd(&__wp, __x);
	          __MCFCRT_memset_piece02_fwd(&__wp, __c);
	          __MCFCRT_memset_piece01_fwd(&__wp, __c);
	          break;
	case 42:  __MCFCRT_memset_piece32_fwd(&__wp, __x);
	          __MCFCRT_memset_piece08_fwd(&__wp, __x);
	          __MCFCRT_memset_piece02_fwd(&__wp, __c);
	          break;
	case 41:  __MCFCRT_memset_piece32_fwd(&__wp, __x);
	          __MCFCRT_memset_piece08_fwd(&__wp, __x);
	          __MCFCRT_memset_piece01_fwd(&__wp, __c);
	          break;
	case 40:  __MCFCRT_memset_piece32_fwd(&__wp, __x);
	          __MCFCRT_memset_piece08_fwd(&__wp, __x);
	          break;
	case 39:  __MCFCRT_memset_piece32_fwd(&__wp, __x);
	          __MCFCRT_memset_piece04_fwd(&__wp, __c);
	          __MCFCRT_memset_piece02_fwd(&__wp, __c);
	          __MCFCRT_memset_piece01_fwd(&__wp, __c);
	          break;
	case 38:  __MCFCRT_memset_piece32_fwd(&__wp, __x);
	          __MCFCRT_memset_piece04_fwd(&__wp, __c);
	          __MCFCRT_memset_piece02_fwd(&__wp, __c);
	          break;
	case 37:  __MCFCRT_memset_piece32_fwd(&__wp, __x);
	          __MCFCRT_memset_piece04_fwd(&__wp, __c);
	          __MCFCRT_memset_piece01_fwd(&__wp, __c);
	          break;
	case 36:  __MCFCRT_memset_piece32_fwd(&__wp, __x);
	          __MCFCRT_memset_piece04_fwd(&__wp, __c);
	          break;
	case 35:  __MCFCRT_memset_piece32_fwd(&__wp, __x);
	          __MCFCRT_memset_piece02_fwd(&__wp, __c);
	          __MCFCRT_memset_piece01_fwd(&__wp, __c);
	          break;
	case 34:  __MCFCRT_memset_piece32_fwd(&__wp, __x);
	          __MCFCRT_memset_piece02_fwd(&__wp, __c);
	          break;
	case 33:  __MCFCRT_memset_piece32_fwd(&__wp, __x);
	          __MCFCRT_memset_piece01_fwd(&__wp, __c);
	          break;
	case 32:  __MCFCRT_memset_piece32_fwd(&__wp, __x);
	          break;
	          // Deal with large blocks.
	default:  if(_MCFCRT_EXPECT((_MCFCRT_STD size_t)(__ewp - __bwp) < 0x100000)){
	            __MCFCRT_memset_large_fwd(__bwp, __ewp, __x);
	          } else {
	            __MCFCRT_memset_huge_fwd(__bwp, __ewp, __x);
	          }
	          break;
	}
}
__MCFCRT_MEMSET_IMPL_INLINE_OR_EXTERN void __MCFCRT_memset_impl_bwd(unsigned char *__bwp, unsigned char *__ewp, _MCFCRT_STD uint32_t __c) _MCFCRT_NOEXCEPT {
	unsigned char *__wp = __ewp;
	__m128 __x = _mm_castsi128_ps(_mm_set1_epi32((int)__c));
	switch((_MCFCRT_STD size_t)(__wp - __bwp)){
	          // Deal with empty blocks.
	case  0:  break;
	          // Deal with tiny blocks using a detailed table.
	case 63:  __MCFCRT_memset_piece01_bwd(&__wp, __c);
	case 62:  __MCFCRT_memset_piece02_bwd(&__wp, __c);
	case 60:  __MCFCRT_memset_piece04_bwd(&__wp, __c);
	case 56:  __MCFCRT_memset_piece08_bwd(&__wp, __x);
	case 48:  __MCFCRT_memset_piece16_bwd(&__wp, __x);
	case 32:  __MCFCRT_memset_piece32_bwd(&__wp, __x);
	          break;
	case 31:  __MCFCRT_memset_piece01_bwd(&__wp, __c);
	case 30:  __MCFCRT_memset_piece02_bwd(&__wp, __c);
	case 28:  __MCFCRT_memset_piece04_bwd(&__wp, __c);
	case 24:  __MCFCRT_memset_piece08_bwd(&__wp, __x);
	case 16:  __MCFCRT_memset_piece16_bwd(&__wp, __x);
	          break;
	case 47:  __MCFCRT_memset_piece01_bwd(&__wp, __c);
	case 46:  __MCFCRT_memset_piece02_bwd(&__wp, __c);
	case 44:  __MCFCRT_memset_piece04_bwd(&__wp, __c);
	case 40:  __MCFCRT_memset_piece08_bwd(&__wp, __x);
	          __MCFCRT_memset_piece32_bwd(&__wp, __x);
	          break;
	case 15:  __MCFCRT_memset_piece01_bwd(&__wp, __c);
	case 14:  __MCFCRT_memset_piece02_bwd(&__wp, __c);
	case 12:  __MCFCRT_memset_piece04_bwd(&__wp, __c);
	case  8:  __MCFCRT_memset_piece08_bwd(&__wp, __x);
	          break;
	case 55:  __MCFCRT_memset_piece01_bwd(&__wp, __c);
	case 54:  __MCFCRT_memset_piece02_bwd(&__wp, __c);
	case 52:  __MCFCRT_memset_piece04_bwd(&__wp, __c);
	          __MCFCRT_memset_piece16_bwd(&__wp, __x);
	          __MCFCRT_memset_piece32_bwd(&__wp, __x);
	          break;
	case 23:  __MCFCRT_memset_piece01_bwd(&__wp, __c);
	case 22:  __MCFCRT_memset_piece02_bwd(&__wp, __c);
	case 20:  __MCFCRT_memset_piece04_bwd(&__wp, __c);
	          __MCFCRT_memset_piece16_bwd(&__wp, __x);
	          break;
	case 39:  __MCFCRT_memset_piece01_bwd(&__wp, __c);
	case 38:  __MCFCRT_memset_piece02_bwd(&__wp, __c);
	case 36:  __MCFCRT_memset_piece04_bwd(&__wp, __c);
	          __MCFCRT_memset_piece32_bwd(&__wp, __x);
	          break;
	case  7:  __MCFCRT_memset_piece01_bwd(&__wp, __c);
	case  6:  __MCFCRT_memset_piece02_bwd(&__wp, __c);
	case  4:  __MCFCRT_memset_piece04_bwd(&__wp, __c);
	          break;
	case 59:  __MCFCRT_memset_piece01_bwd(&__wp, __c);
	case 58:  __MCFCRT_memset_piece02_bwd(&__wp, __c);
	          __MCFCRT_memset_piece08_bwd(&__wp, __x);
	          __MCFCRT_memset_piece16_bwd(&__wp, __x);
	          __MCFCRT_memset_piece32_bwd(&__wp, __x);
	          break;
	case 27:  __MCFCRT_memset_piece01_bwd(&__wp, __c);
	case 26:  __MCFCRT_memset_piece02_bwd(&__wp, __c);
	          __MCFCRT_memset_piece08_bwd(&__wp, __x);
	          __MCFCRT_memset_piece16_bwd(&__wp, __x);
	          break;
	case 43:  __MCFCRT_memset_piece01_bwd(&__wp, __c);
	case 42:  __MCFCRT_memset_piece02_bwd(&__wp, __c);
	          __MCFCRT_memset_piece08_bwd(&__wp, __x);
	          __MCFCRT_memset_piece32_bwd(&__wp, __x);
	          break;
	case 11:  __MCFCRT_memset_piece01_bwd(&__wp, __c);
	case 10:  __MCFCRT_memset_piece02_bwd(&__wp, __c);
	          __MCFCRT_memset_piece08_bwd(&__wp, __x);
	          break;
	case 51:  __MCFCRT_memset_piece01_bwd(&__wp, __c);
	case 50:  __MCFCRT_memset_piece02_bwd(&__wp, __c);
	          __MCFCRT_memset_piece16_bwd(&__wp, __x);
	          __MCFCRT_memset_piece32_bwd(&__wp, __x);
	          break;
	case 19:  __MCFCRT_memset_piece01_bwd(&__wp, __c);
	case 18:  __MCFCRT_memset_piece02_bwd(&__wp, __c);
	          __MCFCRT_memset_piece16_bwd(&__wp, __x);
	          break;
	case 35:  __MCFCRT_memset_piece01_bwd(&__wp, __c);
	case 34:  __MCFCRT_memset_piece02_bwd(&__wp, __c);
	          __MCFCRT_memset_piece32_bwd(&__wp, __x);
	          break;
	case  3:  __MCFCRT_memset_piece01_bwd(&__wp, __c);
	case  2:  __MCFCRT_memset_piece02_bwd(&__wp, __c);
	          break;
	case 61:  __MCFCRT_memset_piece01_bwd(&__wp, __c);
	          __MCFCRT_memset_piece04_bwd(&__wp, __c);
	          __MCFCRT_memset_piece08_bwd(&__wp, __x);
	          __MCFCRT_memset_piece16_bwd(&__wp, __x);
	          __MCFCRT_memset_piece32_bwd(&__wp, __x);
	          break;
	case 29:  __MCFCRT_memset_piece01_bwd(&__wp, __c);
	          __MCFCRT_memset_piece04_bwd(&__wp, __c);
	          __MCFCRT_memset_piece08_bwd(&__wp, __x);
	          __MCFCRT_memset_piece16_bwd(&__wp, __x);
	          break;
	case 45:  __MCFCRT_memset_piece01_bwd(&__wp, __c);
	          __MCFCRT_memset_piece04_bwd(&__wp, __c);
	          __MCFCRT_memset_piece08_bwd(&__wp, __x);
	          __MCFCRT_memset_piece32_bwd(&__wp, __x);
	          break;
	case 13:  __MCFCRT_memset_piece01_bwd(&__wp, __c);
	          __MCFCRT_memset_piece04_bwd(&__wp, __c);
	          __MCFCRT_memset_piece08_bwd(&__wp, __x);
	          break;
	case 53:  __MCFCRT_memset_piece01_bwd(&__wp, __c);
	          __MCFCRT_memset_piece04_bwd(&__wp, __c);
	          __MCFCRT_memset_piece16_bwd(&__wp, __x);
	          __MCFCRT_memset_piece32_bwd(&__wp, __x);
	          break;
	case 21:  __MCFCRT_memset_piece01_bwd(&__wp, __c);
	          __MCFCRT_memset_piece04_bwd(&__wp, __c);
	          __MCFCRT_memset_piece16_bwd(&__wp, __x);
	          break;
	case 37:  __MCFCRT_memset_piece01_bwd(&__wp, __c);
	          __MCFCRT_memset_piece04_bwd(&__wp, __c);
	          __MCFCRT_memset_piece32_bwd(&__wp, __x);
	          break;
	case  5:  __MCFCRT_memset_piece01_bwd(&__wp, __c);
	          __MCFCRT_memset_piece04_bwd(&__wp, __c);
	          break;
	case 57:  __MCFCRT_memset_piece01_bwd(&__wp, __c);
	          __MCFCRT_memset_piece08_bwd(&__wp, __x);
	          __MCFCRT_memset_piece16_bwd(&__wp, __x);
	          __MCFCRT_memset_piece32_bwd(&__wp, __x);
	          break;
	case 25:  __MCFCRT_memset_piece01_bwd(&__wp, __c);
	          __MCFCRT_memset_piece08_bwd(&__wp, __x);
	          __MCFCRT_memset_piece16_bwd(&__wp, __x);
	          break;
	case 41:  __MCFCRT_memset_piece01_bwd(&__wp, __c);
	          __MCFCRT_memset_piece08_bwd(&__wp, __x);
	          __MCFCRT_memset_piece32_bwd(&__wp, __x);
	          break;
	case  9:  __MCFCRT_memset_piece01_bwd(&__wp, __c);
	          __MCFCRT_memset_piece08_bwd(&__wp, __x);
	          break;
	case 49:  __MCFCRT_memset_piece01_bwd(&__wp, __c);
	          __MCFCRT_memset_piece16_bwd(&__wp, __x);
	          __MCFCRT_memset_piece32_bwd(&__wp, __x);
	          break;
	case 17:  __MCFCRT_memset_piece01_bwd(&__wp, __c);
	          __MCFCRT_memset_piece16_bwd(&__wp, __x);
	          break;
	case 33:  __MCFCRT_memset_piece01_bwd(&__wp, __c);
	          __MCFCRT_memset_piece32_bwd(&__wp, __x);
	          break;
	case  1:  __MCFCRT_memset_piece01_bwd(&__wp, __c);
	          break;
	          // Deal with large blocks.
	default:  if(_MCFCRT_EXPECT((_MCFCRT_STD size_t)(__ewp - __bwp) < 0x100000)){
	            __MCFCRT_memset_large_bwd(__bwp, __ewp, __x);
	          } else {
	            __MCFCRT_memset_huge_bwd(__bwp, __ewp, __x);
	          }
	          break;
	}
}

#pragma GCC diagnostic pop

_MCFCRT_EXTERN_C_END

#endif
