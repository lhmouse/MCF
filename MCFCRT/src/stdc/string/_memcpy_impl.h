// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_STDC_STRING_MEMCPY_IMPL_H_
#define __MCFCRT_STDC_STRING_MEMCPY_IMPL_H_

#include "../../env/_crtdef.h"
#include "../../env/expect.h"
#include "../../env/xassert.h"
#include <xmmintrin.h>

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
	_mm_storeu_ps((float *)*__wp, _mm_loadu_ps((const float *)*__rp));
	*__wp += 16;
	*__rp += 16;
}
__attribute__((__gnu_inline__, __always_inline__))
extern inline void __MCFCRT_memcpy_piece16_bwd(unsigned char **_MCFCRT_RESTRICT __wp, const unsigned char **_MCFCRT_RESTRICT __rp) _MCFCRT_NOEXCEPT {
	*__wp -= 16;
	*__rp -= 16;
	_mm_storeu_ps((float *)*__wp, _mm_loadu_ps((const float *)*__rp));
}

__attribute__((__gnu_inline__, __always_inline__))
extern inline void __MCFCRT_memcpy_piece32_fwd(unsigned char **_MCFCRT_RESTRICT __wp, const unsigned char **_MCFCRT_RESTRICT __rp) _MCFCRT_NOEXCEPT {
	// TODO: Rewrite to make use of AVX in the future.
	__MCFCRT_memcpy_piece16_fwd(__wp, __rp);
	__MCFCRT_memcpy_piece16_fwd(__wp, __rp);
}
__attribute__((__gnu_inline__, __always_inline__))
extern inline void __MCFCRT_memcpy_piece32_bwd(unsigned char **_MCFCRT_RESTRICT __wp, const unsigned char **_MCFCRT_RESTRICT __rp) _MCFCRT_NOEXCEPT {
	// TODO: Rewrite to make use of AVX in the future.
	__MCFCRT_memcpy_piece16_bwd(__wp, __rp);
	__MCFCRT_memcpy_piece16_bwd(__wp, __rp);
}

extern void __MCFCRT_memcpy_large_fwd(unsigned char *__bwp, unsigned char *__ewp, const unsigned char *__brp, const unsigned char *__erp) _MCFCRT_NOEXCEPT;
extern void __MCFCRT_memcpy_large_bwd(unsigned char *__bwp, unsigned char *__ewp, const unsigned char *__brp, const unsigned char *__erp) _MCFCRT_NOEXCEPT;

// Dispatchers.
__MCFCRT_MEMCPY_IMPL_INLINE_OR_EXTERN void __MCFCRT_memcpy_impl_fwd(unsigned char *__bwp, unsigned char *__ewp, const unsigned char *__brp, const unsigned char *__erp) _MCFCRT_NOEXCEPT {
	_MCFCRT_ASSERT(__ewp - __bwp == __erp - __brp);
	unsigned char *__wp = __bwp;
	const unsigned char *__rp = __brp;
	switch((_MCFCRT_STD size_t)(__erp - __rp)){
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
	          // Deal with another common size.
	case 64:  __MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece32_fwd(&__wp, &__rp);
	          break;
	          // Deal with large blocks.
	default:  __MCFCRT_memcpy_large_fwd(__bwp, __ewp, __brp, __erp);
	          break;
	}
}
__MCFCRT_MEMCPY_IMPL_INLINE_OR_EXTERN void __MCFCRT_memcpy_impl_bwd(unsigned char *__bwp, unsigned char *__ewp, const unsigned char *__brp, const unsigned char *__erp) _MCFCRT_NOEXCEPT {
	_MCFCRT_ASSERT(__ewp - __bwp == __erp - __brp);
	unsigned char *__wp = __ewp;
	const unsigned char *__rp = __erp;
	switch((_MCFCRT_STD size_t)(__rp - __brp)){
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
	          // Deal with another common size.
	case 64:  __MCFCRT_memcpy_piece32_bwd(&__wp, &__rp);
	          __MCFCRT_memcpy_piece32_bwd(&__wp, &__rp);
	          break;
	          // Deal with large blocks.
	default:  __MCFCRT_memcpy_large_bwd(__bwp, __ewp, __brp, __erp);
	          break;
	}
}

#pragma GCC diagnostic pop

_MCFCRT_EXTERN_C_END

#endif
