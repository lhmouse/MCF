// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_STDC_STRING_SSE3_H_
#define __MCFCRT_STDC_STRING_SSE3_H_

#include "../../env/_crtdef.h"
#include <pmmintrin.h>

_MCFCRT_EXTERN_C_BEGIN

__attribute__((__always_inline__))
static inline void __MCFCRT_xmmsetz(__m128i *__word) _MCFCRT_NOEXCEPT {
	*__word = _mm_setzero_si128();
}
__attribute__((__always_inline__))
static inline void __MCFCRT_xmmsetb(__m128i *__word, _MCFCRT_STD uint8_t __val) _MCFCRT_NOEXCEPT {
	*__word = _mm_set1_epi8((char)__val);
}
__attribute__((__always_inline__))
static inline void __MCFCRT_xmmsetw(__m128i *__word, _MCFCRT_STD uint16_t __val) _MCFCRT_NOEXCEPT {
	*__word = _mm_set1_epi16((short)__val);
}

__attribute__((__always_inline__))
static inline void __MCFCRT_xmmload_n(__m128i (*__loader)(const __m128i *), __m128i *_MCFCRT_RESTRICT __words, const void *_MCFCRT_RESTRICT __src, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
#pragma GCC ivdep
	for(_MCFCRT_STD size_t __i = 0; __i < __n; ++__i){
		__words[__i] = __loader((const __m128i *)__src + __i);
	}
}
__attribute__((__always_inline__))
static inline void __MCFCRT_xmmstore_n(void (*__storer)(__m128i *, __m128i), void *_MCFCRT_RESTRICT __dst, const __m128i *_MCFCRT_RESTRICT __words, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
#pragma GCC ivdep
	for(_MCFCRT_STD size_t __i = 0; __i < __n; ++__i){
		__storer((__m128i *)__dst + __i, __words[__i]);
	}
}

__attribute__((__always_inline__))
static inline void __MCFCRT_xmmchsb_n(__m128i *_MCFCRT_RESTRICT __words, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	const __m128i __mask = _mm_set1_epi8((char)0x80);
#pragma GCC ivdep
	for(_MCFCRT_STD size_t __i = 0; __i < __n; ++__i){
		__words[__i] = _mm_xor_si128(__words[__i], __mask);
	}
}
__attribute__((__always_inline__))
static inline void __MCFCRT_xmmchsw_n(__m128i *_MCFCRT_RESTRICT __words, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	const __m128i __mask = _mm_set1_epi16((short)0x8000);
#pragma GCC ivdep
	for(_MCFCRT_STD size_t __i = 0; __i < __n; ++__i){
		__words[__i] = _mm_xor_si128(__words[__i], __mask);
	}
}

__attribute__((__always_inline__))
static inline _MCFCRT_STD uint32_t __MCFCRT_xmmcmp_21b(const __m128i *__lhs, __m128i *__rhs, __m128i (*__comparator)(__m128i, __m128i)) _MCFCRT_NOEXCEPT {
	__m128i __t = __comparator(__lhs[0], __rhs[0]);
	_MCFCRT_STD uint32_t __mask = (_MCFCRT_STD uint32_t)_mm_movemask_epi8(__t);
	__t = __comparator(__lhs[1], __rhs[0]);
	__mask += (_MCFCRT_STD uint32_t)_mm_movemask_epi8(__t) << 16;
	return __mask;
}
__attribute__((__always_inline__))
static inline _MCFCRT_STD uint32_t __MCFCRT_xmmcmp_41w(const __m128i *__lhs, __m128i *__rhs, __m128i (*__comparator)(__m128i, __m128i)) _MCFCRT_NOEXCEPT {
	__m128i __t = _mm_packs_epi16(__comparator(__lhs[0], __rhs[0]), __comparator(__lhs[1], __rhs[0]));
	_MCFCRT_STD uint32_t __mask = (_MCFCRT_STD uint32_t)_mm_movemask_epi8(__t);
	__t = _mm_packs_epi16(__comparator(__lhs[2], __rhs[0]), __comparator(__lhs[3], __rhs[0]));
	__mask += (_MCFCRT_STD uint32_t)_mm_movemask_epi8(__t) << 16;
	return __mask;
}

__attribute__((__always_inline__))
static inline _MCFCRT_STD uint32_t __MCFCRT_xmmcmp_22b(const __m128i *__lhs, const __m128i *__rhs, __m128i (*__comparator)(__m128i, __m128i)) _MCFCRT_NOEXCEPT {
	__m128i __t = __comparator(__lhs[0], __rhs[0]);
	_MCFCRT_STD uint32_t __mask = (_MCFCRT_STD uint32_t)_mm_movemask_epi8(__t);
	__t = __comparator(__lhs[1], __rhs[1]);
	__mask += (_MCFCRT_STD uint32_t)_mm_movemask_epi8(__t) << 16;
	return __mask;
}
__attribute__((__always_inline__))
static inline _MCFCRT_STD uint32_t __MCFCRT_xmmcmp_44w(const __m128i *__lhs, const __m128i *__rhs, __m128i (*__comparator)(__m128i, __m128i)) _MCFCRT_NOEXCEPT {
	__m128i __t = _mm_packs_epi16(__comparator(__lhs[0], __rhs[0]), __comparator(__lhs[1], __rhs[1]));
	_MCFCRT_STD uint32_t __mask = (_MCFCRT_STD uint32_t)_mm_movemask_epi8(__t);
	__t = _mm_packs_epi16(__comparator(__lhs[2], __rhs[2]), __comparator(__lhs[3], __rhs[3]));
	__mask += (_MCFCRT_STD uint32_t)_mm_movemask_epi8(__t) << 16;
	return __mask;
}

_MCFCRT_EXTERN_C_END

#endif
