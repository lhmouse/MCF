// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_STDC_STRING_MEMCPY_IMPL_H_
#define __MCFCRT_STDC_STRING_MEMCPY_IMPL_H_

#include "../../env/_crtdef.h"
#include "../../env/expect.h"

_MCFCRT_EXTERN_C_BEGIN

_MCFCRT_CONSTEXPR bool __MCFCRT_memcpy_is_small_enough(_MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	return __n <= 256;
}

static inline void __MCFCRT_memcpy_small_fwd(void *__s1, const void *__s2, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	unsigned char *__wp = (unsigned char *)__s1;
	const unsigned char *__rp = (const unsigned char *)__s2;
	size_t __rem = __n / sizeof(_MCFCRT_STD uintptr_t);
	if(_MCFCRT_EXPECT_NOT(__rem != 0)){
		switch(__rem % 8){
			_MCFCRT_STD uintptr_t __w;
			do {
#define __MCFCRT_STEP_	\
				__builtin_memcpy(&__w, __rp, sizeof(__w));	\
				__builtin_memcpy(__wp, &__w, sizeof(__w));	\
				__wp += sizeof(__w);	\
				__rp += sizeof(__w);	\
				--__rem;
//=============================================================================
		__attribute__((__fallthrough__)); default: __MCFCRT_STEP_
		__attribute__((__fallthrough__)); case 7:  __MCFCRT_STEP_
		__attribute__((__fallthrough__)); case 6:  __MCFCRT_STEP_
		__attribute__((__fallthrough__)); case 5:  __MCFCRT_STEP_
		__attribute__((__fallthrough__)); case 4:  __MCFCRT_STEP_
		__attribute__((__fallthrough__)); case 3:  __MCFCRT_STEP_
		__attribute__((__fallthrough__)); case 2:  __MCFCRT_STEP_
		__attribute__((__fallthrough__)); case 1:  __MCFCRT_STEP_
//=============================================================================
#undef __MCFCRT_STEP_
			} while(_MCFCRT_EXPECT(__rem != 0));
		}
	}
	__rem = (_MCFCRT_STD size_t)((const unsigned char *)__s1 + __n - __wp);
	if(_MCFCRT_EXPECT(__rem != 0)){
		switch(__rem % 8){
			do {
#define __MCFCRT_STEP_	\
				*(volatile unsigned char *)(__wp++) = *(__rp++);	\
				--__rem;
//=============================================================================
		__attribute__((__fallthrough__)); default: __MCFCRT_STEP_
		__attribute__((__fallthrough__)); case 7:  __MCFCRT_STEP_
		__attribute__((__fallthrough__)); case 6:  __MCFCRT_STEP_
		__attribute__((__fallthrough__)); case 5:  __MCFCRT_STEP_
		__attribute__((__fallthrough__)); case 4:  __MCFCRT_STEP_
		__attribute__((__fallthrough__)); case 3:  __MCFCRT_STEP_
		__attribute__((__fallthrough__)); case 2:  __MCFCRT_STEP_
		__attribute__((__fallthrough__)); case 1:  __MCFCRT_STEP_
//=============================================================================
#undef __MCFCRT_STEP_
			} while(_MCFCRT_EXPECT_NOT(__rem != 0));
		}
	}
}

void __MCFCRT_memcpy_large_fwd(void *__s1, const void *__s2, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT;

static inline void __MCFCRT_memcpy_small_bwd(void *__s1, const void *__s2, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT {
	unsigned char *__wp = (unsigned char *)__s1 + __n;
	const unsigned char *__rp = (const unsigned char *)__s2 + __n;
	size_t __rem = __n / sizeof(_MCFCRT_STD uintptr_t);
	if(_MCFCRT_EXPECT_NOT(__rem != 0)){
		switch(__rem % 8){
			_MCFCRT_STD uintptr_t __w;
			do {
#define __MCFCRT_STEP_	\
				__wp -= sizeof(__w);	\
				__rp -= sizeof(__w);	\
				__builtin_memcpy(&__w, __rp, sizeof(__w));	\
				__builtin_memcpy(__wp, &__w, sizeof(__w));	\
				--__rem;
//=============================================================================
		__attribute__((__fallthrough__)); default: __MCFCRT_STEP_
		__attribute__((__fallthrough__)); case 7:  __MCFCRT_STEP_
		__attribute__((__fallthrough__)); case 6:  __MCFCRT_STEP_
		__attribute__((__fallthrough__)); case 5:  __MCFCRT_STEP_
		__attribute__((__fallthrough__)); case 4:  __MCFCRT_STEP_
		__attribute__((__fallthrough__)); case 3:  __MCFCRT_STEP_
		__attribute__((__fallthrough__)); case 2:  __MCFCRT_STEP_
		__attribute__((__fallthrough__)); case 1:  __MCFCRT_STEP_
//=============================================================================
#undef __MCFCRT_STEP_
			} while(_MCFCRT_EXPECT(__rem != 0));
		}
	}
	__rem = (_MCFCRT_STD size_t)(__wp - (const unsigned char *)__s1);
	if(_MCFCRT_EXPECT(__rem != 0)){
		switch(__rem % 8){
			do {
#define __MCFCRT_STEP_	\
				*(volatile unsigned char *)(--__wp) = *(--__rp);	\
				--__rem;
//=============================================================================
		__attribute__((__fallthrough__)); default: __MCFCRT_STEP_
		__attribute__((__fallthrough__)); case 7:  __MCFCRT_STEP_
		__attribute__((__fallthrough__)); case 6:  __MCFCRT_STEP_
		__attribute__((__fallthrough__)); case 5:  __MCFCRT_STEP_
		__attribute__((__fallthrough__)); case 4:  __MCFCRT_STEP_
		__attribute__((__fallthrough__)); case 3:  __MCFCRT_STEP_
		__attribute__((__fallthrough__)); case 2:  __MCFCRT_STEP_
		__attribute__((__fallthrough__)); case 1:  __MCFCRT_STEP_
//=============================================================================
#undef __MCFCRT_STEP_
			} while(_MCFCRT_EXPECT_NOT(__rem != 0));
		}
	}
}

void __MCFCRT_memcpy_large_bwd(void *__s1, const void *__s2, _MCFCRT_STD size_t __n) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
