// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_STDC_STRING_SSE2_H_
#define __MCFCRT_STDC_STRING_SSE2_H_

#include "../../env/_crtdef.h"
#include <emmintrin.h>

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
static inline const void *__MCFCRT_xmmload_2(__m128i *_MCFCRT_RESTRICT __words, const void *_MCFCRT_RESTRICT __src, __m128i (*__loader)(const __m128i *)) _MCFCRT_NOEXCEPT {
	__m128i *__wp = __words;
	const __m128i *__rp = (const __m128i *)__src;
	for(unsigned __i = 0; __i < 2; ++__i){
		*(__wp++) = __loader(__rp++);
	}
	return __rp;
}
__attribute__((__always_inline__))
static inline const void *__MCFCRT_xmmload_4(__m128i *_MCFCRT_RESTRICT __words, const void *_MCFCRT_RESTRICT __src, __m128i (*__loader)(const __m128i *)) _MCFCRT_NOEXCEPT {
	__m128i *__wp = __words;
	const __m128i *__rp = (const __m128i *)__src;
	for(unsigned __i = 0; __i < 4; ++__i){
		*(__wp++) = __loader(__rp++);
	}
	return __rp;
}

__attribute__((__always_inline__))
static inline void *__MCFCRT_xmmstore_2(void *_MCFCRT_RESTRICT __dst, const __m128i *_MCFCRT_RESTRICT __words, void (*__storer)(__m128i *, __m128i)) _MCFCRT_NOEXCEPT {
	__m128i *__wp = (__m128i *)__dst;
	const __m128i *__rp = __words;
	for(unsigned __i = 0; __i < 2; ++__i){
		__storer(__wp++, *(__rp++));
	}
	return __wp;
}
__attribute__((__always_inline__))
static inline void *__MCFCRT_xmmstore_4(void *_MCFCRT_RESTRICT __dst, const __m128i *_MCFCRT_RESTRICT __words, void (*__storer)(__m128i *, __m128i)) _MCFCRT_NOEXCEPT {
	__m128i *__wp = (__m128i *)__dst;
	const __m128i *__rp = __words;
	for(unsigned __i = 0; __i < 4; ++__i){
		__storer(__wp++, *(__rp++));
	}
	return __wp;
}

__attribute__((__always_inline__))
static inline void __MCFCRT_xmmchs_2b(__m128i *_MCFCRT_RESTRICT __words) _MCFCRT_NOEXCEPT {
	const __m128i __mask = _mm_set1_epi8((char)0x80);
	for(unsigned __i = 0; __i < 2; ++__i){
		__words[__i] = _mm_xor_si128(__words[__i], __mask);
	}
}
__attribute__((__always_inline__))
static inline void __MCFCRT_xmmchs_4w(__m128i *_MCFCRT_RESTRICT __words) _MCFCRT_NOEXCEPT {
	const __m128i __mask = _mm_set1_epi16((short)0x8000);
	for(unsigned __i = 0; __i < 4; ++__i){
		__words[__i] = _mm_xor_si128(__words[__i], __mask);
	}
}

__attribute__((__always_inline__))
static inline _MCFCRT_STD uint32_t __MCFCRT_xmmcmp_21b(const __m128i *__lhs, __m128i *__rhs, __m128i (*__comp)(__m128i, __m128i)) _MCFCRT_NOEXCEPT {
	_MCFCRT_STD uint32_t __mask = 0;
	for(unsigned __i = 0; __i < 2; ++__i){
		const __m128i __t = __comp(__lhs[__i], __rhs[0]);
		__mask += (_MCFCRT_STD uint32_t)_mm_movemask_epi8(__t) << __i * 16;
	}
	return __mask;
}
__attribute__((__always_inline__))
static inline _MCFCRT_STD uint32_t __MCFCRT_xmmcmp_41w(const __m128i *__lhs, __m128i *__rhs, __m128i (*__comp)(__m128i, __m128i)) _MCFCRT_NOEXCEPT {
	_MCFCRT_STD uint32_t __mask = 0;
	for(unsigned __i = 0; __i < 2; ++__i){
		const __m128i __t0 = __comp(__lhs[__i * 2 + 0], __rhs[0]);
		const __m128i __t1 = __comp(__lhs[__i * 2 + 1], __rhs[0]);
		__mask += (_MCFCRT_STD uint32_t)_mm_movemask_epi8(_mm_packs_epi16(__t0, __t1)) << __i * 16;
	}
	return __mask;
}

__attribute__((__always_inline__))
static inline _MCFCRT_STD uint32_t __MCFCRT_xmmcmp_22b(const __m128i *__lhs, const __m128i *__rhs, __m128i (*__comp)(__m128i, __m128i)) _MCFCRT_NOEXCEPT {
	_MCFCRT_STD uint32_t __mask = 0;
	for(unsigned __i = 0; __i < 2; ++__i){
		const __m128i __t = __comp(__lhs[__i], __rhs[__i]);
		__mask += (_MCFCRT_STD uint32_t)_mm_movemask_epi8(__t) << __i * 16;
	}
	return __mask;
}
__attribute__((__always_inline__))
static inline _MCFCRT_STD uint32_t __MCFCRT_xmmcmp_44w(const __m128i *__lhs, const __m128i *__rhs, __m128i (*__comp)(__m128i, __m128i)) _MCFCRT_NOEXCEPT {
	_MCFCRT_STD uint32_t __mask = 0;
	for(unsigned __i = 0; __i < 2; ++__i){
		const __m128i __t0 = __comp(__lhs[__i * 2 + 0], __rhs[__i * 2 + 0]);
		const __m128i __t1 = __comp(__lhs[__i * 2 + 1], __rhs[__i * 2 + 1]);
		__mask += (_MCFCRT_STD uint32_t)_mm_movemask_epi8(_mm_packs_epi16(__t0, __t1)) << __i * 16;
	}
	return __mask;
}

__attribute__((__always_inline__))
static inline _MCFCRT_STD uint32_t __MCFCRT_xmmcmpor_211b(const __m128i *__lhs, const __m128i *__rhs, const __m128i *__third, __m128i (*__comp)(__m128i, __m128i)) _MCFCRT_NOEXCEPT {
	_MCFCRT_STD uint32_t __mask = 0;
	for(unsigned __i = 0; __i < 2; ++__i){
		const __m128i __t = _mm_or_si128(__comp(__lhs[__i], __rhs[0]), __comp(__lhs[__i], __third[0]));
		__mask += (_MCFCRT_STD uint32_t)_mm_movemask_epi8(__t) << __i * 16;
	}
	return __mask;
}
__attribute__((__always_inline__))
static inline _MCFCRT_STD uint32_t __MCFCRT_xmmcmpor_411w(const __m128i *__lhs, const __m128i *__rhs, const __m128i *__third, __m128i (*__comp)(__m128i, __m128i)) _MCFCRT_NOEXCEPT {
	_MCFCRT_STD uint32_t __mask = 0;
	for(unsigned __i = 0; __i < 2; ++__i){
		const __m128i __t0 = _mm_or_si128(__comp(__lhs[__i * 2 + 0], __rhs[0]), __comp(__lhs[__i * 2 + 0], __third[0]));
		const __m128i __t1 = _mm_or_si128(__comp(__lhs[__i * 2 + 1], __rhs[0]), __comp(__lhs[__i * 2 + 1], __third[0]));
		__mask += (_MCFCRT_STD uint32_t)_mm_movemask_epi8(_mm_packs_epi16(__t0, __t1)) << __i * 16;
	}
	return __mask;
}

__attribute__((__always_inline__))
static inline _MCFCRT_STD uint32_t __MCFCRT_xmmcmpor_221b(const __m128i *__lhs, const __m128i *__rhs, const __m128i *__third, __m128i (*__comp)(__m128i, __m128i)) _MCFCRT_NOEXCEPT {
	_MCFCRT_STD uint32_t __mask = 0;
	for(unsigned __i = 0; __i < 2; ++__i){
		const __m128i __t = _mm_or_si128(__comp(__lhs[__i], __rhs[__i]), __comp(__lhs[__i], __third[0]));
		__mask += (_MCFCRT_STD uint32_t)_mm_movemask_epi8(__t) << __i * 16;
	}
	return __mask;
}
__attribute__((__always_inline__))
static inline _MCFCRT_STD uint32_t __MCFCRT_xmmcmpor_441w(const __m128i *__lhs, const __m128i *__rhs, const __m128i *__third, __m128i (*__comp)(__m128i, __m128i)) _MCFCRT_NOEXCEPT {
	_MCFCRT_STD uint32_t __mask = 0;
	for(unsigned __i = 0; __i < 2; ++__i){
		const __m128i __t0 = _mm_or_si128(__comp(__lhs[__i * 2 + 0], __rhs[__i * 2 + 0]), __comp(__lhs[__i * 2 + 0], __third[0]));
		const __m128i __t1 = _mm_or_si128(__comp(__lhs[__i * 2 + 1], __rhs[__i * 2 + 1]), __comp(__lhs[__i * 2 + 1], __third[0]));
		__mask += (_MCFCRT_STD uint32_t)_mm_movemask_epi8(_mm_packs_epi16(__t0, __t1)) << __i * 16;
	}
	return __mask;
}

_MCFCRT_EXTERN_C_END

#endif
