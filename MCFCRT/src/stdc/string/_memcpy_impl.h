// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_STDC_STRING_MEMCPY_IMPL_H_
#define __MCFCRT_STDC_STRING_MEMCPY_IMPL_H_

#include "../../env/_crtdef.h"
#include "../../env/expect.h"
#include <emmintrin.h>

_MCFCRT_EXTERN_C_BEGIN

_MCFCRT_CONSTEXPR static inline bool __MCFCRT_memcpy_is_small_enough(_MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	return __n < 128;
}

static inline void __MCFCRT_memcpy_small_fwd(void *__s1, const void *__s2, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	register unsigned char *__wp __asm__("cx") = (unsigned char *)__s1;
	register const unsigned char *__rp __asm__("si") = (const unsigned char *)__s2;
	_MCFCRT_STD size_t __rem = __n / 8;
	if(_MCFCRT_EXPECT_NOT(__rem != 0)){
		switch((__rem - 1) % 8){
#define __MCFCRT_STEP_(__k_)	\
				__attribute__((__fallthrough__));	\
		case (__k_):	\
				_mm_storel_epi64((__m128i *)__wp, _mm_loadl_epi64((const __m128i *)__rp));	\
				__wp += 8;	\
				__rp += 8;	\
				--__rem;
//=============================================================================
			do {
		__MCFCRT_STEP_(7)
		__MCFCRT_STEP_(6)
		__MCFCRT_STEP_(5)
		__MCFCRT_STEP_(4)
		__MCFCRT_STEP_(3)
		__MCFCRT_STEP_(2)
		__MCFCRT_STEP_(1)
		__MCFCRT_STEP_(0)
			} while(_MCFCRT_EXPECT(__rem != 0));
//=============================================================================
#undef __MCFCRT_STEP_
		}
	}
	__rem = __n % 8;
#define __MCFCRT_STEP_(__b_)	\
	if(_MCFCRT_EXPECT(__rem & (__b_ / 8))){	\
		*(volatile _MCFCRT_STD uint##__b_##_t *)__wp = *(_MCFCRT_STD uint##__b_##_t *)__rp;	\
		__wp += __b_ / 8;	\
		__rp += __b_ / 8;	\
	}
//=============================================================================
	__MCFCRT_STEP_(32)
	__MCFCRT_STEP_(16)
	__MCFCRT_STEP_( 8)
//=============================================================================
#undef __MCFCRT_STEP_
}

void __MCFCRT_memcpy_large_fwd(void *__s1, const void *__s2, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT;

static inline void __MCFCRT_memcpy_small_bwd(_MCFCRT_STD size_t __n, void *__s1, const void *__s2) _MCFCRT_NOEXCEPT {
	register unsigned char *__wp __asm__("dx") = (unsigned char *)__s1;
	register const unsigned char *__rp __asm__("si") = (const unsigned char *)__s2;
	_MCFCRT_STD size_t __rem = __n % 8;
#define __MCFCRT_STEP_(__b_)	\
	if(_MCFCRT_EXPECT(__rem & (__b_ / 8))){	\
		__wp -= __b_ / 8;	\
		__rp -= __b_ / 8;	\
		*(volatile _MCFCRT_STD uint##__b_##_t *)__wp = *(_MCFCRT_STD uint##__b_##_t *)__rp;	\
	}
//=============================================================================
	__MCFCRT_STEP_(32)
	__MCFCRT_STEP_(16)
	__MCFCRT_STEP_( 8)
//=============================================================================
#undef __MCFCRT_STEP_
	__rem = __n / 8;
	if(_MCFCRT_EXPECT_NOT(__rem != 0)){
		switch((__rem - 1) % 8){
#define __MCFCRT_STEP_(__k_)	\
				__attribute__((__fallthrough__));	\
		case (__k_):	\
				__wp -= 8;	\
				__rp -= 8;	\
				_mm_storel_epi64((__m128i *)__wp, _mm_loadl_epi64((const __m128i *)__rp));	\
				--__rem;
//=============================================================================
			do {
		__MCFCRT_STEP_(7)
		__MCFCRT_STEP_(6)
		__MCFCRT_STEP_(5)
		__MCFCRT_STEP_(4)
		__MCFCRT_STEP_(3)
		__MCFCRT_STEP_(2)
		__MCFCRT_STEP_(1)
		__MCFCRT_STEP_(0)
			} while(_MCFCRT_EXPECT(__rem != 0));
//=============================================================================
#undef __MCFCRT_STEP_
		}
	}
}

void __MCFCRT_memcpy_large_bwd(_MCFCRT_STD size_t __n, void *__s1, const void *__s2) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
