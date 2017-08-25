// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "_memcpy_impl.h"
#include "../../env/expect.h"
#include "_sse2.h"

void __MCFCRT_memcpy_large_fwd(void *s1, const void *s2, size_t n){
	register unsigned char *wp __asm__("di") = (unsigned char *)s1;
	register const unsigned char *rp __asm__("si") = (const unsigned char *)s2;
	size_t rem = n / 16;
	if(_MCFCRT_EXPECT_NOT(rem >= 2)){
		while(((uintptr_t)wp & ~(uintptr_t)-16) != 0){
			*(volatile unsigned char *)(wp++) = *(rp++);
		}
#define STEP(store_, load_)	\
		store_((float *)wp, load_((const float *)rp));	\
		wp += 16;	\
		rp += 16;	\
		--rem;
#define FULL(store_, load_)	\
		switch(rem % 8){	\
			do {	\
		__attribute__((__fallthrough__)); default: STEP(store_, load_)	\
		__attribute__((__fallthrough__)); case 7:  STEP(store_, load_)	\
		__attribute__((__fallthrough__)); case 6:  STEP(store_, load_)	\
		__attribute__((__fallthrough__)); case 5:  STEP(store_, load_)	\
		__attribute__((__fallthrough__)); case 4:  STEP(store_, load_)	\
		__attribute__((__fallthrough__)); case 3:  STEP(store_, load_)	\
		__attribute__((__fallthrough__)); case 2:  STEP(store_, load_)	\
		__attribute__((__fallthrough__)); case 1:  STEP(store_, load_)	\
			} while(_MCFCRT_EXPECT(rem != 0));	\
		}
//=============================================================================
		if(_MCFCRT_EXPECT(n < 0x40000)){
			if(((uintptr_t)rp & ~(uintptr_t)-16) == 0){
				FULL(_mm_store_ps, _mm_load_ps)
			} else {
				FULL(_mm_store_ps, _mm_loadu_ps)
			}
		} else {
			if(((uintptr_t)rp & ~(uintptr_t)-16) == 0){
				FULL(_mm_stream_ps, _mm_load_ps)
			} else {
				FULL(_mm_stream_ps, _mm_loadu_ps)
			}
			_mm_sfence();
		}
//=============================================================================
#undef STEP
#undef FULL
	}
	rem = (size_t)((const unsigned char *)s1 + n - wp);
	while(rem != 0){
		*(volatile unsigned char *)(wp++) = *(rp++);
		--rem;
	}
}

void __MCFCRT_memcpy_large_bwd(void *s1, const void *s2, size_t n){
	register unsigned char *wp __asm__("di") = (unsigned char *)s1;
	register const unsigned char *rp __asm__("si") = (const unsigned char *)s2;
	size_t rem = n / 16;
	if(_MCFCRT_EXPECT_NOT(rem >= 2)){
		while(((uintptr_t)wp & ~(uintptr_t)-16) != 0){
			*(volatile unsigned char *)(--wp) = *(--rp);
		}
#define STEP(store_, load_)	\
		wp -= 16;	\
		rp -= 16;	\
		store_((float *)wp, load_((const float *)rp));	\
		--rem;
#define FULL(store_, load_)	\
		switch(rem % 8){	\
			do {	\
		__attribute__((__fallthrough__)); default: STEP(store_, load_)	\
		__attribute__((__fallthrough__)); case 7:  STEP(store_, load_)	\
		__attribute__((__fallthrough__)); case 6:  STEP(store_, load_)	\
		__attribute__((__fallthrough__)); case 5:  STEP(store_, load_)	\
		__attribute__((__fallthrough__)); case 4:  STEP(store_, load_)	\
		__attribute__((__fallthrough__)); case 3:  STEP(store_, load_)	\
		__attribute__((__fallthrough__)); case 2:  STEP(store_, load_)	\
		__attribute__((__fallthrough__)); case 1:  STEP(store_, load_)	\
			} while(_MCFCRT_EXPECT(rem != 0));	\
		}
//=============================================================================
		if(_MCFCRT_EXPECT(n < 0x40000)){
			if(((uintptr_t)rp & ~(uintptr_t)-16) == 0){
				FULL(_mm_store_ps, _mm_load_ps)
			} else {
				FULL(_mm_store_ps, _mm_loadu_ps)
			}
		} else {
			if(((uintptr_t)rp & ~(uintptr_t)-16) == 0){
				FULL(_mm_stream_ps, _mm_load_ps)
			} else {
				FULL(_mm_stream_ps, _mm_loadu_ps)
			}
			_mm_sfence();
		}
//=============================================================================
#undef STEP
#undef FULL
	}
	rem = (size_t)((const unsigned char *)s1 + n - wp);
	while(rem != 0){
		*(volatile unsigned char *)(--wp) = *(--rp);
		--rem;
	}
}
