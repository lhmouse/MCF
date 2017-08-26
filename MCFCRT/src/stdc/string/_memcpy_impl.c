// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "_memcpy_impl.h"
#include "../../env/xassert.h"

void __MCFCRT_memcpy_large_fwd(void *s1, const void *s2, size_t n){
	_MCFCRT_ASSERT(n >= 16);

	register unsigned char *wp __asm__("cx") = (unsigned char *)s1;
	register const unsigned char *rp __asm__("si") = (const unsigned char *)s2;
	const size_t off = -(uintptr_t)s1 & ~(uintptr_t)-16; // off = 16 - misalignment
	__m128i mis_w;
	if(_MCFCRT_EXPECT(off != 0)){
		mis_w = _mm_loadu_si128((const __m128i *)s2);
		wp += off;
		rp += off;
	}
	size_t rem = (n - off) / 16;
	if(_MCFCRT_EXPECT(rem != 0)){
		const size_t nt = !!(n >> 18) << 4;
		const size_t ur = !!((uintptr_t)rp & ~(uintptr_t)-16) << 3;
		switch(rem % 8 + nt + ur){
#define STEP(case_, store_, load_)	\
				__attribute__((__fallthrough__));	\
		case_:	\
				store_((__m128i *)wp, load_((const __m128i *)rp));	\
				wp += 16;	\
				rp += 16;	\
				--rem;
//=============================================================================
			break;
		// temporal, aligned read
			do {
		STEP(case 000, _mm_store_si128 , _mm_load_si128 )
		STEP(case 007, _mm_store_si128 , _mm_load_si128 )
		STEP(case 006, _mm_store_si128 , _mm_load_si128 )
		STEP(case 005, _mm_store_si128 , _mm_load_si128 )
		STEP(case 004, _mm_store_si128 , _mm_load_si128 )
		STEP(case 003, _mm_store_si128 , _mm_load_si128 )
		STEP(case 002, _mm_store_si128 , _mm_load_si128 )
		STEP(case 001, _mm_store_si128 , _mm_load_si128 )
			} while(_MCFCRT_EXPECT(rem != 0));
//=============================================================================
			break;
		// temporal, unaligned read
			do {
		STEP(case 010, _mm_store_si128 , _mm_loadu_si128)
		STEP(case 017, _mm_store_si128 , _mm_loadu_si128)
		STEP(case 016, _mm_store_si128 , _mm_loadu_si128)
		STEP(case 015, _mm_store_si128 , _mm_loadu_si128)
		STEP(case 014, _mm_store_si128 , _mm_loadu_si128)
		STEP(case 013, _mm_store_si128 , _mm_loadu_si128)
		STEP(case 012, _mm_store_si128 , _mm_loadu_si128)
		STEP(case 011, _mm_store_si128 , _mm_loadu_si128)
			} while(_MCFCRT_EXPECT(rem != 0));
//=============================================================================
			break;
		// non-temporal, aligned read
			do {
		STEP(case 020, _mm_stream_si128, _mm_load_si128 )
		STEP(case 027, _mm_stream_si128, _mm_load_si128 )
		STEP(case 026, _mm_stream_si128, _mm_load_si128 )
		STEP(case 025, _mm_stream_si128, _mm_load_si128 )
		STEP(case 024, _mm_stream_si128, _mm_load_si128 )
		STEP(case 023, _mm_stream_si128, _mm_load_si128 )
		STEP(case 022, _mm_stream_si128, _mm_load_si128 )
		STEP(case 021, _mm_stream_si128, _mm_load_si128 )
			} while(_MCFCRT_EXPECT(rem != 0));
//=============================================================================
			break;
		// non-temporal, unaligned read
			do {
		STEP(case 030, _mm_stream_si128, _mm_loadu_si128)
		STEP(case 037, _mm_stream_si128, _mm_loadu_si128)
		STEP(case 036, _mm_stream_si128, _mm_loadu_si128)
		STEP(case 035, _mm_stream_si128, _mm_loadu_si128)
		STEP(case 034, _mm_stream_si128, _mm_loadu_si128)
		STEP(case 033, _mm_stream_si128, _mm_loadu_si128)
		STEP(case 032, _mm_stream_si128, _mm_loadu_si128)
		STEP(case 031, _mm_stream_si128, _mm_loadu_si128)
			} while(_MCFCRT_EXPECT(rem != 0));
//=============================================================================
#undef STEP
		}
	}
	rem = (n - off) % 16;
#define STEP(b_)	\
	if(_MCFCRT_EXPECT(rem & (b_ / 8))){	\
		*(volatile uint ## b_ ## _t *)wp = *(uint ## b_ ## _t *)rp;	\
		wp += b_ / 8;	\
		rp += b_ / 8;	\
	}
//=============================================================================
	STEP(64)
	STEP(32)
	STEP(16)
	STEP( 8)
//=============================================================================
#undef STEP
	if(_MCFCRT_EXPECT(off != 0)){
		_mm_storeu_si128((__m128i *)s1, mis_w);
	}
}

void __MCFCRT_memcpy_large_bwd(size_t n, void *s1, const void *s2){
	_MCFCRT_ASSERT(n >= 16);

	register unsigned char *wp __asm__("dx") = (unsigned char *)s1;
	register const unsigned char *rp __asm__("si") = (const unsigned char *)s2;
	const size_t off = (uintptr_t)s1 & ~(uintptr_t)-16; // off = misalignment
	__m128i mis_w;
	if(_MCFCRT_EXPECT(off != 0)){
		mis_w = _mm_loadu_si128((const __m128i *)s2 - 1);
		wp -= off;
		rp -= off;
	}
	size_t rem = (n - off) / 16;
	if(_MCFCRT_EXPECT(rem != 0)){
		const size_t nt = !!(n >> 18) << 4;
		const size_t ur = !!((uintptr_t)rp & ~(uintptr_t)-16) << 3;
		switch(rem % 8 + nt + ur){
#define STEP(case_, store_, load_)	\
				__attribute__((__fallthrough__));	\
		case_:	\
				wp -= 16;	\
				rp -= 16;	\
				store_((__m128i *)wp, load_((const __m128i *)rp));	\
				--rem;
//=============================================================================
			break;
		// temporal, aligned read
			do {
		STEP(case 000, _mm_store_si128 , _mm_load_si128 )
		STEP(case 007, _mm_store_si128 , _mm_load_si128 )
		STEP(case 006, _mm_store_si128 , _mm_load_si128 )
		STEP(case 005, _mm_store_si128 , _mm_load_si128 )
		STEP(case 004, _mm_store_si128 , _mm_load_si128 )
		STEP(case 003, _mm_store_si128 , _mm_load_si128 )
		STEP(case 002, _mm_store_si128 , _mm_load_si128 )
		STEP(case 001, _mm_store_si128 , _mm_load_si128 )
			} while(_MCFCRT_EXPECT(rem != 0));
//=============================================================================
			break;
		// temporal, unaligned read
			do {
		STEP(case 010, _mm_store_si128 , _mm_loadu_si128)
		STEP(case 017, _mm_store_si128 , _mm_loadu_si128)
		STEP(case 016, _mm_store_si128 , _mm_loadu_si128)
		STEP(case 015, _mm_store_si128 , _mm_loadu_si128)
		STEP(case 014, _mm_store_si128 , _mm_loadu_si128)
		STEP(case 013, _mm_store_si128 , _mm_loadu_si128)
		STEP(case 012, _mm_store_si128 , _mm_loadu_si128)
		STEP(case 011, _mm_store_si128 , _mm_loadu_si128)
			} while(_MCFCRT_EXPECT(rem != 0));
//=============================================================================
			break;
		// non-temporal, aligned read
			do {
		STEP(case 020, _mm_stream_si128, _mm_load_si128 )
		STEP(case 027, _mm_stream_si128, _mm_load_si128 )
		STEP(case 026, _mm_stream_si128, _mm_load_si128 )
		STEP(case 025, _mm_stream_si128, _mm_load_si128 )
		STEP(case 024, _mm_stream_si128, _mm_load_si128 )
		STEP(case 023, _mm_stream_si128, _mm_load_si128 )
		STEP(case 022, _mm_stream_si128, _mm_load_si128 )
		STEP(case 021, _mm_stream_si128, _mm_load_si128 )
			} while(_MCFCRT_EXPECT(rem != 0));
//=============================================================================
			break;
		// non-temporal, unaligned read
			do {
		STEP(case 030, _mm_stream_si128, _mm_loadu_si128)
		STEP(case 037, _mm_stream_si128, _mm_loadu_si128)
		STEP(case 036, _mm_stream_si128, _mm_loadu_si128)
		STEP(case 035, _mm_stream_si128, _mm_loadu_si128)
		STEP(case 034, _mm_stream_si128, _mm_loadu_si128)
		STEP(case 033, _mm_stream_si128, _mm_loadu_si128)
		STEP(case 032, _mm_stream_si128, _mm_loadu_si128)
		STEP(case 031, _mm_stream_si128, _mm_loadu_si128)
			} while(_MCFCRT_EXPECT(rem != 0));
//=============================================================================
#undef STEP
		}
	}
	rem = (n - off) % 16;
#define STEP(b_)	\
	if(_MCFCRT_EXPECT(rem & (b_ / 8))){	\
		wp -= b_ / 8;	\
		rp -= b_ / 8;	\
		*(volatile uint ## b_ ## _t *)wp = *(uint ## b_ ## _t *)rp;	\
	}
//=============================================================================
	STEP(64)
	STEP(32)
	STEP(16)
	STEP( 8)
//=============================================================================
#undef STEP
	if(_MCFCRT_EXPECT(off != 0)){
		_mm_storeu_si128((__m128i *)s1 - 1, mis_w);
	}
}
