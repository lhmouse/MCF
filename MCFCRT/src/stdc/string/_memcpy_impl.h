// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_STDC_STRING_MEMCPY_IMPL_H_
#define __MCFCRT_STDC_STRING_MEMCPY_IMPL_H_

#include "../../env/_crtdef.h"
#include "../../env/expect.h"
#include "../../env/xassert.h"
#include <emmintrin.h>

#ifndef __MCFCRT_MEMCPY_IMPL_INLINE_OR_EXTERN
#  define __MCFCRT_MEMCPY_IMPL_INLINE_OR_EXTERN     __attribute__((__gnu_inline__)) extern inline
#endif

_MCFCRT_EXTERN_C_BEGIN

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-unreachable"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"

__attribute__((__gnu_inline__, __always_inline__))
extern inline void __MCFCRT_memcpy_piece01_fwd(unsigned char **_MCFCRT_RESTRICT __wp, const unsigned char **_MCFCRT_RESTRICT __rp) _MCFCRT_NOEXCEPT {
	*(volatile _MCFCRT_STD uint8_t *)*__wp = *(const _MCFCRT_STD uint8_t *)*__rp;
	*__wp += 1;
	*__rp += 1;
}
__attribute__((__gnu_inline__, __always_inline__))
extern inline void __MCFCRT_memcpy_piece01_bwd(unsigned char **_MCFCRT_RESTRICT __wp, const unsigned char **_MCFCRT_RESTRICT __rp) _MCFCRT_NOEXCEPT {
	*__wp -= 1;
	*__rp -= 1;
	*(volatile _MCFCRT_STD uint8_t *)*__wp = *(const _MCFCRT_STD uint8_t *)*__rp;
}

__attribute__((__gnu_inline__, __always_inline__))
extern inline void __MCFCRT_memcpy_piece02_fwd(unsigned char **_MCFCRT_RESTRICT __wp, const unsigned char **_MCFCRT_RESTRICT __rp) _MCFCRT_NOEXCEPT {
	*(volatile _MCFCRT_STD uint16_t *)*__wp = *(const _MCFCRT_STD uint16_t *)*__rp;
	*__wp += 2;
	*__rp += 2;
}
__attribute__((__gnu_inline__, __always_inline__))
extern inline void __MCFCRT_memcpy_piece02_bwd(unsigned char **_MCFCRT_RESTRICT __wp, const unsigned char **_MCFCRT_RESTRICT __rp) _MCFCRT_NOEXCEPT {
	*__wp -= 2;
	*__rp -= 2;
	*(volatile _MCFCRT_STD uint16_t *)*__wp = *(const _MCFCRT_STD uint16_t *)*__rp;
}

__attribute__((__gnu_inline__, __always_inline__))
extern inline void __MCFCRT_memcpy_piece04_fwd(unsigned char **_MCFCRT_RESTRICT __wp, const unsigned char **_MCFCRT_RESTRICT __rp) _MCFCRT_NOEXCEPT {
	*(volatile _MCFCRT_STD uint32_t *)*__wp = *(const _MCFCRT_STD uint32_t *)*__rp;
	*__wp += 4;
	*__rp += 4;
}
__attribute__((__gnu_inline__, __always_inline__))
extern inline void __MCFCRT_memcpy_piece04_bwd(unsigned char **_MCFCRT_RESTRICT __wp, const unsigned char **_MCFCRT_RESTRICT __rp) _MCFCRT_NOEXCEPT {
	*__wp -= 4;
	*__rp -= 4;
	*(volatile _MCFCRT_STD uint32_t *)*__wp = *(const _MCFCRT_STD uint32_t *)*__rp;
}

__attribute__((__gnu_inline__, __always_inline__))
extern inline void __MCFCRT_memcpy_piece08_fwd(unsigned char **_MCFCRT_RESTRICT __wp, const unsigned char **_MCFCRT_RESTRICT __rp) _MCFCRT_NOEXCEPT {
	*(volatile _MCFCRT_STD uint64_t *)*__wp = *(const _MCFCRT_STD uint64_t *)*__rp;
	*__wp += 8;
	*__rp += 8;
}
__attribute__((__gnu_inline__, __always_inline__))
extern inline void __MCFCRT_memcpy_piece08_bwd(unsigned char **_MCFCRT_RESTRICT __wp, const unsigned char **_MCFCRT_RESTRICT __rp) _MCFCRT_NOEXCEPT {
	*__wp -= 8;
	*__rp -= 8;
	*(volatile _MCFCRT_STD uint64_t *)*__wp = *(const _MCFCRT_STD uint64_t *)*__rp;
}

__attribute__((__gnu_inline__, __always_inline__))
extern inline void __MCFCRT_memcpy_piece16_fwd(unsigned char **_MCFCRT_RESTRICT __wp, const unsigned char **_MCFCRT_RESTRICT __rp) _MCFCRT_NOEXCEPT {
	_mm_storeu_si128((__m128i *)*__wp, _mm_loadu_si128((const __m128i *)*__rp));
	*__wp += 16;
	*__rp += 16;
}
__attribute__((__gnu_inline__, __always_inline__))
extern inline void __MCFCRT_memcpy_piece16_bwd(unsigned char **_MCFCRT_RESTRICT __wp, const unsigned char **_MCFCRT_RESTRICT __rp) _MCFCRT_NOEXCEPT {
	*__wp -= 16;
	*__rp -= 16;
	_mm_storeu_si128((__m128i *)*__wp, _mm_loadu_si128((const __m128i *)*__rp));
}

__attribute__((__gnu_inline__, __always_inline__))
extern inline void __MCFCRT_memcpy_piece32_fwd(unsigned char **_MCFCRT_RESTRICT __wp, const unsigned char **_MCFCRT_RESTRICT __rp) _MCFCRT_NOEXCEPT {
	// TODO: Rewrite to make use of AVX in the future.
	_mm_storeu_si128((__m128i *)*__wp, _mm_loadu_si128((const __m128i *)*__rp));
	*__wp += 16;
	*__rp += 16;
	_mm_storeu_si128((__m128i *)*__wp, _mm_loadu_si128((const __m128i *)*__rp));
	*__wp += 16;
	*__rp += 16;
}
__attribute__((__gnu_inline__, __always_inline__))
extern inline void __MCFCRT_memcpy_piece32_bwd(unsigned char **_MCFCRT_RESTRICT __wp, const unsigned char **_MCFCRT_RESTRICT __rp) _MCFCRT_NOEXCEPT {
	// TODO: Rewrite to make use of AVX in the future.
	*__wp -= 16;
	*__rp -= 16;
	_mm_storeu_si128((__m128i *)*__wp, _mm_loadu_si128((const __m128i *)*__rp));
	*__wp -= 16;
	*__rp -= 16;
	_mm_storeu_si128((__m128i *)*__wp, _mm_loadu_si128((const __m128i *)*__rp));
}

__attribute__((__gnu_inline__, __always_inline__))
extern inline void __MCFCRT_memcpy_aligned32_fwd(unsigned char **_MCFCRT_RESTRICT __wp, const unsigned char **_MCFCRT_RESTRICT __rp) _MCFCRT_NOEXCEPT {
	// TODO: Rewrite to make use of AVX in the future.
	_mm_store_si128((__m128i *)*__wp, _mm_loadu_si128((const __m128i *)*__rp));
	*__wp += 16;
	*__rp += 16;
	_mm_store_si128((__m128i *)*__wp, _mm_loadu_si128((const __m128i *)*__rp));
	*__wp += 16;
	*__rp += 16;
}
__attribute__((__gnu_inline__, __always_inline__))
extern inline void __MCFCRT_memcpy_aligned32_bwd(unsigned char **_MCFCRT_RESTRICT __wp, const unsigned char **_MCFCRT_RESTRICT __rp) _MCFCRT_NOEXCEPT {
	// TODO: Rewrite to make use of AVX in the future.
	*__wp -= 16;
	*__rp -= 16;
	_mm_store_si128((__m128i *)*__wp, _mm_loadu_si128((const __m128i *)*__rp));
	*__wp -= 16;
	*__rp -= 16;
	_mm_store_si128((__m128i *)*__wp, _mm_loadu_si128((const __m128i *)*__rp));
}

__attribute__((__gnu_inline__, __always_inline__))
extern inline void __MCFCRT_memcpy_nontemp32_fwd(unsigned char **_MCFCRT_RESTRICT __wp, const unsigned char **_MCFCRT_RESTRICT __rp) _MCFCRT_NOEXCEPT {
	// TODO: Rewrite to make use of AVX in the future.
	_mm_stream_si128((__m128i *)*__wp, _mm_loadu_si128((const __m128i *)*__rp));
	*__wp += 16;
	*__rp += 16;
	_mm_stream_si128((__m128i *)*__wp, _mm_loadu_si128((const __m128i *)*__rp));
	*__wp += 16;
	*__rp += 16;
}
__attribute__((__gnu_inline__, __always_inline__))
extern inline void __MCFCRT_memcpy_nontemp32_bwd(unsigned char **_MCFCRT_RESTRICT __wp, const unsigned char **_MCFCRT_RESTRICT __rp) _MCFCRT_NOEXCEPT {
	// TODO: Rewrite to make use of AVX in the future.
	*__wp -= 16;
	*__rp -= 16;
	_mm_stream_si128((__m128i *)*__wp, _mm_loadu_si128((const __m128i *)*__rp));
	*__wp -= 16;
	*__rp -= 16;
	_mm_stream_si128((__m128i *)*__wp, _mm_loadu_si128((const __m128i *)*__rp));
}

// Functions that copy blocks no smaller than 64 bytes.
__MCFCRT_MEMCPY_IMPL_INLINE_OR_EXTERN void __MCFCRT_memcpy_large_fwd(unsigned char *__bwp, unsigned char *__ewp, const unsigned char *__brp, const unsigned char *__erp){
	_MCFCRT_ASSERT(__ewp - __bwp == __erp - __brp);
	_MCFCRT_ASSERT(__ewp - __bwp >= 64);
	// Stash potentially unaligned QQWORDs in both ends of the source range.
	// They might be subsequently clobbered if the source and destination ranges overlap.
	unsigned char __stash[64];
	unsigned char *__wp = __stash;
	const unsigned char *__rp = __brp;
	__MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	__wp = __stash + 64;
	__rp = __erp;
	__MCFCRT_memcpy_piece32_bwd(&__wp, &__rp);
	// If there is misalignment at all, align the write pointer to 32-byte boundaries, rounding downwards.
	// We will copy the final QQWORD separately, hence the last (0,32] bytes are excluded here.
	__wp = (unsigned char *)((_MCFCRT_STD uintptr_t)(__bwp + 32) & (_MCFCRT_STD uintptr_t)-32);
	__rp = __erp - (__ewp - __wp);
	// Copy QQWORDs to aligned locations.
	switch((_MCFCRT_STD size_t)(__ewp - __wp - 1) / 32 % 16){
		do {
#define __MCFCRT_COPY_STEP_(k_)	\
		__MCFCRT_memcpy_aligned32_fwd(&__wp, &__rp);	\
	case (k_):	\
		;
//=============================================================================
	__MCFCRT_COPY_STEP_(017)  __MCFCRT_COPY_STEP_(016)  __MCFCRT_COPY_STEP_(015)  __MCFCRT_COPY_STEP_(014)
	__MCFCRT_COPY_STEP_(013)  __MCFCRT_COPY_STEP_(012)  __MCFCRT_COPY_STEP_(011)  __MCFCRT_COPY_STEP_(010)
	__MCFCRT_COPY_STEP_(007)  __MCFCRT_COPY_STEP_(006)  __MCFCRT_COPY_STEP_(005)  __MCFCRT_COPY_STEP_(004)
	__MCFCRT_COPY_STEP_(003)  __MCFCRT_COPY_STEP_(002)  __MCFCRT_COPY_STEP_(001)  __MCFCRT_COPY_STEP_(000)
//=============================================================================
#undef __MCFCRT_COPY_STEP_
		} while(_MCFCRT_EXPECT((_MCFCRT_STD size_t)(__ewp - __wp) > 32));
	}
	// Pop the stash.
	__wp = __bwp;
	__rp = __stash;
	__MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	__wp = __ewp;
	__rp = __stash + 64;
	__MCFCRT_memcpy_piece32_bwd(&__wp, &__rp);
}
__MCFCRT_MEMCPY_IMPL_INLINE_OR_EXTERN void __MCFCRT_memcpy_large_bwd(unsigned char *__bwp, unsigned char *__ewp, const unsigned char *__brp, const unsigned char *__erp){
	_MCFCRT_ASSERT(__ewp - __bwp == __erp - __brp);
	_MCFCRT_ASSERT(__ewp - __bwp >= 64);
	// Stash potentially unaligned QQWORDs in both ends of the source range.
	// They might be subsequently clobbered if the source and destination ranges overlap.
	unsigned char __stash[64];
	unsigned char *__wp = __stash;
	const unsigned char *__rp = __brp;
	__MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	__wp = __stash + 64;
	__rp = __erp;
	__MCFCRT_memcpy_piece32_bwd(&__wp, &__rp);
	// If there is misalignment at all, align the write pointer to 32-byte boundaries, rounding downwards.
	// We will copy the initial QQWORD separately, hence the last (0,32] bytes are excluded here.
	__wp = (unsigned char *)((_MCFCRT_STD uintptr_t)(__ewp - 1) & (_MCFCRT_STD uintptr_t)-32);
	__rp = __brp + (__wp - __bwp);
	// Copy QQWORDs to aligned locations.
	switch((_MCFCRT_STD size_t)(__wp - __bwp - 1) / 32 % 16){
		do {
#define __MCFCRT_COPY_STEP_(k_)	\
		__MCFCRT_memcpy_aligned32_bwd(&__wp, &__rp);	\
	case (k_):	\
		;
//=============================================================================
	__MCFCRT_COPY_STEP_(017)  __MCFCRT_COPY_STEP_(016)  __MCFCRT_COPY_STEP_(015)  __MCFCRT_COPY_STEP_(014)
	__MCFCRT_COPY_STEP_(013)  __MCFCRT_COPY_STEP_(012)  __MCFCRT_COPY_STEP_(011)  __MCFCRT_COPY_STEP_(010)
	__MCFCRT_COPY_STEP_(007)  __MCFCRT_COPY_STEP_(006)  __MCFCRT_COPY_STEP_(005)  __MCFCRT_COPY_STEP_(004)
	__MCFCRT_COPY_STEP_(003)  __MCFCRT_COPY_STEP_(002)  __MCFCRT_COPY_STEP_(001)  __MCFCRT_COPY_STEP_(000)
//=============================================================================
#undef __MCFCRT_COPY_STEP_
		} while(_MCFCRT_EXPECT((_MCFCRT_STD size_t)(__wp - __bwp) > 32));
	}
	// Pop the stash.
	__wp = __bwp;
	__rp = __stash;
	__MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	__wp = __ewp;
	__rp = __stash + 64;
	__MCFCRT_memcpy_piece32_bwd(&__wp, &__rp);
}

// Functions that copy blocks no smaller than 64 bytes using non-temporal semantics.
__MCFCRT_MEMCPY_IMPL_INLINE_OR_EXTERN void __MCFCRT_memcpy_huge_fwd(unsigned char *__bwp, unsigned char *__ewp, const unsigned char *__brp, const unsigned char *__erp){
	_MCFCRT_ASSERT(__ewp - __bwp == __erp - __brp);
	_MCFCRT_ASSERT(__ewp - __bwp >= 64);
	// Stash potentially unaligned QQWORDs in both ends of the source range.
	// They might be subsequently clobbered if the source and destination ranges overlap.
	unsigned char __stash[64];
	unsigned char *__wp = __stash;
	const unsigned char *__rp = __brp;
	__MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	__wp = __stash + 64;
	__rp = __erp;
	__MCFCRT_memcpy_piece32_bwd(&__wp, &__rp);
	// If there is misalignment at all, align the write pointer to 32-byte boundaries, rounding downwards.
	// We will copy the final QQWORD separately, hence the last (0,32] bytes are excluded here.
	__wp = (unsigned char *)((_MCFCRT_STD uintptr_t)(__bwp + 32) & (_MCFCRT_STD uintptr_t)-32);
	__rp = __erp - (__ewp - __wp);
	// Copy QQWORDs to aligned locations.
	switch((_MCFCRT_STD size_t)(__ewp - __wp - 1) / 32 % 16){
		do {
#define __MCFCRT_COPY_STEP_(k_)	\
		__MCFCRT_memcpy_nontemp32_fwd(&__wp, &__rp);	\
	case (k_):	\
		;
//=============================================================================
	__MCFCRT_COPY_STEP_(017)  __MCFCRT_COPY_STEP_(016)  __MCFCRT_COPY_STEP_(015)  __MCFCRT_COPY_STEP_(014)
	__MCFCRT_COPY_STEP_(013)  __MCFCRT_COPY_STEP_(012)  __MCFCRT_COPY_STEP_(011)  __MCFCRT_COPY_STEP_(010)
	__MCFCRT_COPY_STEP_(007)  __MCFCRT_COPY_STEP_(006)  __MCFCRT_COPY_STEP_(005)  __MCFCRT_COPY_STEP_(004)
	__MCFCRT_COPY_STEP_(003)  __MCFCRT_COPY_STEP_(002)  __MCFCRT_COPY_STEP_(001)  __MCFCRT_COPY_STEP_(000)
//=============================================================================
#undef __MCFCRT_COPY_STEP_
		} while(_MCFCRT_EXPECT((_MCFCRT_STD size_t)(__ewp - __wp) > 32));
	}
	// Don't forget the a store fence.
	_mm_sfence();
	// Pop the stash.
	__wp = __bwp;
	__rp = __stash;
	__MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	__wp = __ewp;
	__rp = __stash + 64;
	__MCFCRT_memcpy_piece32_bwd(&__wp, &__rp);
}
__MCFCRT_MEMCPY_IMPL_INLINE_OR_EXTERN void __MCFCRT_memcpy_huge_bwd(unsigned char *__bwp, unsigned char *__ewp, const unsigned char *__brp, const unsigned char *__erp){
	_MCFCRT_ASSERT(__ewp - __bwp == __erp - __brp);
	_MCFCRT_ASSERT(__ewp - __bwp >= 64);
	// Stash potentially unaligned QQWORDs in both ends of the source range.
	// They might be subsequently clobbered if the source and destination ranges overlap.
	unsigned char __stash[64];
	unsigned char *__wp = __stash;
	const unsigned char *__rp = __brp;
	__MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	__wp = __stash + 64;
	__rp = __erp;
	__MCFCRT_memcpy_piece32_bwd(&__wp, &__rp);
	// If there is misalignment at all, align the write pointer to 32-byte boundaries, rounding downwards.
	// We will copy the initial QQWORD separately, hence the last (0,32] bytes are excluded here.
	__wp = (unsigned char *)((_MCFCRT_STD uintptr_t)(__ewp - 1) & (_MCFCRT_STD uintptr_t)-32);
	__rp = __brp + (__wp - __bwp);
	// Copy QQWORDs to aligned locations.
	switch((_MCFCRT_STD size_t)(__wp - __bwp - 1) / 32 % 16){
		do {
#define __MCFCRT_COPY_STEP_(k_)	\
		__MCFCRT_memcpy_nontemp32_bwd(&__wp, &__rp);	\
	case (k_):	\
		;
//=============================================================================
	__MCFCRT_COPY_STEP_(017)  __MCFCRT_COPY_STEP_(016)  __MCFCRT_COPY_STEP_(015)  __MCFCRT_COPY_STEP_(014)
	__MCFCRT_COPY_STEP_(013)  __MCFCRT_COPY_STEP_(012)  __MCFCRT_COPY_STEP_(011)  __MCFCRT_COPY_STEP_(010)
	__MCFCRT_COPY_STEP_(007)  __MCFCRT_COPY_STEP_(006)  __MCFCRT_COPY_STEP_(005)  __MCFCRT_COPY_STEP_(004)
	__MCFCRT_COPY_STEP_(003)  __MCFCRT_COPY_STEP_(002)  __MCFCRT_COPY_STEP_(001)  __MCFCRT_COPY_STEP_(000)
//=============================================================================
#undef __MCFCRT_COPY_STEP_
		} while(_MCFCRT_EXPECT((_MCFCRT_STD size_t)(__wp - __bwp) > 32));
	}
	// Don't forget the a store fence.
	_mm_sfence();
	// Pop the stash.
	__wp = __bwp;
	__rp = __stash;
	__MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	__wp = __ewp;
	__rp = __stash + 64;
	__MCFCRT_memcpy_piece32_bwd(&__wp, &__rp);
}

// Dispatchers.
__MCFCRT_MEMCPY_IMPL_INLINE_OR_EXTERN void __MCFCRT_memcpy_impl_fwd(unsigned char *__bwp, unsigned char *__ewp, const unsigned char *__brp, const unsigned char *__erp) _MCFCRT_NOEXCEPT {
	_MCFCRT_ASSERT(__ewp - __bwp == __erp - __brp);
	unsigned char *__wp = __bwp;
	const unsigned char *__rp = __brp;
	switch((_MCFCRT_STD size_t)(__ewp - __wp)){
	          // Deal with empty blocks.
	case  0:  break;
	          // Deal with tiny blocks using a detailed table.
	case 63:  __MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	case 31:  __MCFCRT_memcpy_piece16_fwd(&__wp, &__rp);
	case 15:  __MCFCRT_memcpy_piece08_fwd(&__wp, &__rp);
	case  7:  __MCFCRT_memcpy_piece04_fwd(&__wp, &__rp);
	case  3:  __MCFCRT_memcpy_piece02_fwd(&__wp, &__rp);
	case  1:  __MCFCRT_memcpy_piece01_fwd(&__wp, &__rp);
	          break;
	case 62:  __MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	case 30:  __MCFCRT_memcpy_piece16_fwd(&__wp, &__rp);
	case 14:  __MCFCRT_memcpy_piece08_fwd(&__wp, &__rp);
	case  6:  __MCFCRT_memcpy_piece04_fwd(&__wp, &__rp);
	case  2:  __MCFCRT_memcpy_piece02_fwd(&__wp, &__rp);
	          break;
	case 61:  __MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	case 29:  __MCFCRT_memcpy_piece16_fwd(&__wp, &__rp);
	case 13:  __MCFCRT_memcpy_piece08_fwd(&__wp, &__rp);
	case  5:  __MCFCRT_memcpy_piece04_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece01_fwd(&__wp, &__rp);
	          break;
	case 60:  __MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	case 28:  __MCFCRT_memcpy_piece16_fwd(&__wp, &__rp);
	case 12:  __MCFCRT_memcpy_piece08_fwd(&__wp, &__rp);
	case  4:  __MCFCRT_memcpy_piece04_fwd(&__wp, &__rp);
	          break;
	case 59:  __MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	case 27:  __MCFCRT_memcpy_piece16_fwd(&__wp, &__rp);
	case 11:  __MCFCRT_memcpy_piece08_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece02_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece01_fwd(&__wp, &__rp);
	          break;
	case 58:  __MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	case 26:  __MCFCRT_memcpy_piece16_fwd(&__wp, &__rp);
	case 10:  __MCFCRT_memcpy_piece08_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece02_fwd(&__wp, &__rp);
	          break;
	case 57:  __MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	case 25:  __MCFCRT_memcpy_piece16_fwd(&__wp, &__rp);
	case  9:  __MCFCRT_memcpy_piece08_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece01_fwd(&__wp, &__rp);
	          break;
	case 56:  __MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	case 24:  __MCFCRT_memcpy_piece16_fwd(&__wp, &__rp);
	case  8:  __MCFCRT_memcpy_piece08_fwd(&__wp, &__rp);
	          break;
	case 55:  __MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	case 23:  __MCFCRT_memcpy_piece16_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece04_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece02_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece01_fwd(&__wp, &__rp);
	          break;
	case 54:  __MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	case 22:  __MCFCRT_memcpy_piece16_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece04_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece02_fwd(&__wp, &__rp);
	          break;
	case 53:  __MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	case 21:  __MCFCRT_memcpy_piece16_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece04_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece01_fwd(&__wp, &__rp);
	          break;
	case 52:  __MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	case 20:  __MCFCRT_memcpy_piece16_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece04_fwd(&__wp, &__rp);
	          break;
	case 51:  __MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	case 19:  __MCFCRT_memcpy_piece16_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece02_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece01_fwd(&__wp, &__rp);
	          break;
	case 50:  __MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	case 18:  __MCFCRT_memcpy_piece16_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece02_fwd(&__wp, &__rp);
	          break;
	case 49:  __MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	case 17:  __MCFCRT_memcpy_piece16_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece01_fwd(&__wp, &__rp);
	          break;
	case 48:  __MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	case 16:  __MCFCRT_memcpy_piece16_fwd(&__wp, &__rp);
	          break;
	case 47:  __MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece08_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece04_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece02_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece01_fwd(&__wp, &__rp);
	          break;
	case 46:  __MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece08_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece04_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece02_fwd(&__wp, &__rp);
	          break;
	case 45:  __MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece08_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece04_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece01_fwd(&__wp, &__rp);
	          break;
	case 44:  __MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece08_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece04_fwd(&__wp, &__rp);
	          break;
	case 43:  __MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece08_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece02_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece01_fwd(&__wp, &__rp);
	          break;
	case 42:  __MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece08_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece02_fwd(&__wp, &__rp);
	          break;
	case 41:  __MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece08_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece01_fwd(&__wp, &__rp);
	          break;
	case 40:  __MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece08_fwd(&__wp, &__rp);
	          break;
	case 39:  __MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece04_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece02_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece01_fwd(&__wp, &__rp);
	          break;
	case 38:  __MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece04_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece02_fwd(&__wp, &__rp);
	          break;
	case 37:  __MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece04_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece01_fwd(&__wp, &__rp);
	          break;
	case 36:  __MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece04_fwd(&__wp, &__rp);
	          break;
	case 35:  __MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece02_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece01_fwd(&__wp, &__rp);
	          break;
	case 34:  __MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece02_fwd(&__wp, &__rp);
	          break;
	case 33:  __MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece01_fwd(&__wp, &__rp);
	          break;
	case 32:  __MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	          break;
	case 64:  __MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	          break;
	          // Deal with large blocks.
	default:  if(_MCFCRT_EXPECT((_MCFCRT_STD size_t)(__ewp - __bwp) <= 0x200000)){
	            __MCFCRT_memcpy_large_fwd(__bwp, __ewp, __brp, __erp);
	          } else {
	            __MCFCRT_memcpy_huge_fwd(__bwp, __ewp, __brp, __erp);
	          }
	          break;
	}
}
__MCFCRT_MEMCPY_IMPL_INLINE_OR_EXTERN void __MCFCRT_memcpy_impl_bwd(unsigned char *__bwp, unsigned char *__ewp, const unsigned char *__brp, const unsigned char *__erp) _MCFCRT_NOEXCEPT {
	_MCFCRT_ASSERT(__ewp - __bwp == __erp - __brp);
	unsigned char *__wp = __ewp;
	const unsigned char *__rp = __erp;
	switch((_MCFCRT_STD size_t)(__wp - __bwp)){
	          // Deal with empty blocks.
	case  0:  break;
	          // Deal with tiny blocks using a detailed table.
	case 63:  __MCFCRT_memcpy_piece01_bwd(&__wp, &__rp);
	case 62:  __MCFCRT_memcpy_piece02_bwd(&__wp, &__rp);
	case 60:  __MCFCRT_memcpy_piece04_bwd(&__wp, &__rp);
	case 56:  __MCFCRT_memcpy_piece08_bwd(&__wp, &__rp);
	case 48:  __MCFCRT_memcpy_piece16_bwd(&__wp, &__rp);
	case 32:  __MCFCRT_memcpy_piece32_bwd(&__wp, &__rp);
	          break;
	case 31:  __MCFCRT_memcpy_piece01_bwd(&__wp, &__rp);
	case 30:  __MCFCRT_memcpy_piece02_bwd(&__wp, &__rp);
	case 28:  __MCFCRT_memcpy_piece04_bwd(&__wp, &__rp);
	case 24:  __MCFCRT_memcpy_piece08_bwd(&__wp, &__rp);
	case 16:  __MCFCRT_memcpy_piece16_bwd(&__wp, &__rp);
	          break;
	case 47:  __MCFCRT_memcpy_piece01_bwd(&__wp, &__rp);
	case 46:  __MCFCRT_memcpy_piece02_bwd(&__wp, &__rp);
	case 44:  __MCFCRT_memcpy_piece04_bwd(&__wp, &__rp);
	case 40:  __MCFCRT_memcpy_piece08_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece32_bwd(&__wp, &__rp);
	          break;
	case 15:  __MCFCRT_memcpy_piece01_bwd(&__wp, &__rp);
	case 14:  __MCFCRT_memcpy_piece02_bwd(&__wp, &__rp);
	case 12:  __MCFCRT_memcpy_piece04_bwd(&__wp, &__rp);
	case  8:  __MCFCRT_memcpy_piece08_bwd(&__wp, &__rp);
	          break;
	case 55:  __MCFCRT_memcpy_piece01_bwd(&__wp, &__rp);
	case 54:  __MCFCRT_memcpy_piece02_bwd(&__wp, &__rp);
	case 52:  __MCFCRT_memcpy_piece04_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece16_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece32_bwd(&__wp, &__rp);
	          break;
	case 23:  __MCFCRT_memcpy_piece01_bwd(&__wp, &__rp);
	case 22:  __MCFCRT_memcpy_piece02_bwd(&__wp, &__rp);
	case 20:  __MCFCRT_memcpy_piece04_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece16_bwd(&__wp, &__rp);
	          break;
	case 39:  __MCFCRT_memcpy_piece01_bwd(&__wp, &__rp);
	case 38:  __MCFCRT_memcpy_piece02_bwd(&__wp, &__rp);
	case 36:  __MCFCRT_memcpy_piece04_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece32_bwd(&__wp, &__rp);
	          break;
	case  7:  __MCFCRT_memcpy_piece01_bwd(&__wp, &__rp);
	case  6:  __MCFCRT_memcpy_piece02_bwd(&__wp, &__rp);
	case  4:  __MCFCRT_memcpy_piece04_bwd(&__wp, &__rp);
	          break;
	case 59:  __MCFCRT_memcpy_piece01_bwd(&__wp, &__rp);
	case 58:  __MCFCRT_memcpy_piece02_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece08_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece16_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece32_bwd(&__wp, &__rp);
	          break;
	case 27:  __MCFCRT_memcpy_piece01_bwd(&__wp, &__rp);
	case 26:  __MCFCRT_memcpy_piece02_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece08_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece16_bwd(&__wp, &__rp);
	          break;
	case 43:  __MCFCRT_memcpy_piece01_bwd(&__wp, &__rp);
	case 42:  __MCFCRT_memcpy_piece02_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece08_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece32_bwd(&__wp, &__rp);
	          break;
	case 11:  __MCFCRT_memcpy_piece01_bwd(&__wp, &__rp);
	case 10:  __MCFCRT_memcpy_piece02_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece08_bwd(&__wp, &__rp);
	          break;
	case 51:  __MCFCRT_memcpy_piece01_bwd(&__wp, &__rp);
	case 50:  __MCFCRT_memcpy_piece02_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece16_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece32_bwd(&__wp, &__rp);
	          break;
	case 19:  __MCFCRT_memcpy_piece01_bwd(&__wp, &__rp);
	case 18:  __MCFCRT_memcpy_piece02_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece16_bwd(&__wp, &__rp);
	          break;
	case 35:  __MCFCRT_memcpy_piece01_bwd(&__wp, &__rp);
	case 34:  __MCFCRT_memcpy_piece02_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece32_bwd(&__wp, &__rp);
	          break;
	case  3:  __MCFCRT_memcpy_piece01_bwd(&__wp, &__rp);
	case  2:  __MCFCRT_memcpy_piece02_bwd(&__wp, &__rp);
	          break;
	case 61:  __MCFCRT_memcpy_piece01_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece04_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece08_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece16_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece32_bwd(&__wp, &__rp);
	          break;
	case 29:  __MCFCRT_memcpy_piece01_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece04_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece08_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece16_bwd(&__wp, &__rp);
	          break;
	case 45:  __MCFCRT_memcpy_piece01_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece04_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece08_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece32_bwd(&__wp, &__rp);
	          break;
	case 13:  __MCFCRT_memcpy_piece01_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece04_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece08_bwd(&__wp, &__rp);
	          break;
	case 53:  __MCFCRT_memcpy_piece01_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece04_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece16_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece32_bwd(&__wp, &__rp);
	          break;
	case 21:  __MCFCRT_memcpy_piece01_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece04_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece16_bwd(&__wp, &__rp);
	          break;
	case 37:  __MCFCRT_memcpy_piece01_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece04_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece32_bwd(&__wp, &__rp);
	          break;
	case  5:  __MCFCRT_memcpy_piece01_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece04_bwd(&__wp, &__rp);
	          break;
	case 57:  __MCFCRT_memcpy_piece01_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece08_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece16_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece32_bwd(&__wp, &__rp);
	          break;
	case 25:  __MCFCRT_memcpy_piece01_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece08_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece16_bwd(&__wp, &__rp);
	          break;
	case 41:  __MCFCRT_memcpy_piece01_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece08_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece32_bwd(&__wp, &__rp);
	          break;
	case  9:  __MCFCRT_memcpy_piece01_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece08_bwd(&__wp, &__rp);
	          break;
	case 49:  __MCFCRT_memcpy_piece01_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece16_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece32_bwd(&__wp, &__rp);
	          break;
	case 17:  __MCFCRT_memcpy_piece01_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece16_bwd(&__wp, &__rp);
	          break;
	case 33:  __MCFCRT_memcpy_piece01_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece32_bwd(&__wp, &__rp);
	          break;
	case  1:  __MCFCRT_memcpy_piece01_bwd(&__wp, &__rp);
	          break;
	case 64:  __MCFCRT_memcpy_piece32_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece32_bwd(&__wp, &__rp);
	          break;
	          // Deal with large blocks.
	default:  if(_MCFCRT_EXPECT((_MCFCRT_STD size_t)(__ewp - __bwp) <= 0x200000)){
	            __MCFCRT_memcpy_large_bwd(__bwp, __ewp, __brp, __erp);
	          } else {
	            __MCFCRT_memcpy_huge_bwd(__bwp, __ewp, __brp, __erp);
	          }
	          break;
	}
}

#pragma GCC diagnostic pop

_MCFCRT_EXTERN_C_END

#endif
