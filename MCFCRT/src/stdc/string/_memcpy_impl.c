// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "_memcpy_impl.h"
#include "../../env/xassert.h"

void __MCFCRT_memcpy_large_fwd(void *s1, const void *s2, size_t n){
	_MCFCRT_ASSERT(n >= 16);

	unsigned char *wp = (unsigned char *)s1;
	const unsigned char *rp = (const unsigned char *)s2;
	const size_t off = -(uintptr_t)s1 % 16; // off = 16 - misalignment
	__m128i mis_w;
	if(_MCFCRT_EXPECT(off != 0)){
		mis_w = _mm_loadu_si128((const __m128i *)s2);
		wp += off;
		rp += off;
	}
	size_t rem = (n - off) / 16;
	if(_MCFCRT_EXPECT(rem != 0)){
		const size_t nt = !!(n >> 20) << 4;
		const size_t ur = !!((uintptr_t)rp % 16) << 3;
		switch((rem - 1) % 8 + nt + ur){
#define STEP(k_, store_, load_)	\
				__attribute__((__fallthrough__));	\
		case (k_):	\
				store_((__m128i *)wp, load_((const __m128i *)rp));	\
				wp += 16;	\
				rp += 16;	\
				--rem;
//=============================================================================
			break;
		// temporal, aligned read
			do {
		STEP(007, _mm_store_si128 , _mm_load_si128 )
		STEP(006, _mm_store_si128 , _mm_load_si128 )
		STEP(005, _mm_store_si128 , _mm_load_si128 )
		STEP(004, _mm_store_si128 , _mm_load_si128 )
		STEP(003, _mm_store_si128 , _mm_load_si128 )
		STEP(002, _mm_store_si128 , _mm_load_si128 )
		STEP(001, _mm_store_si128 , _mm_load_si128 )
		STEP(000, _mm_store_si128 , _mm_load_si128 )
			} while(_MCFCRT_EXPECT(rem != 0));
//=============================================================================
			break;
		// temporal, unaligned read
			do {
		STEP(017, _mm_store_si128 , _mm_loadu_si128)
		STEP(016, _mm_store_si128 , _mm_loadu_si128)
		STEP(015, _mm_store_si128 , _mm_loadu_si128)
		STEP(014, _mm_store_si128 , _mm_loadu_si128)
		STEP(013, _mm_store_si128 , _mm_loadu_si128)
		STEP(012, _mm_store_si128 , _mm_loadu_si128)
		STEP(011, _mm_store_si128 , _mm_loadu_si128)
		STEP(010, _mm_store_si128 , _mm_loadu_si128)
			} while(_MCFCRT_EXPECT(rem != 0));
//=============================================================================
			break;
		// non-temporal, aligned read
			do {
		STEP(027, _mm_stream_si128, _mm_load_si128 )
		STEP(026, _mm_stream_si128, _mm_load_si128 )
		STEP(025, _mm_stream_si128, _mm_load_si128 )
		STEP(024, _mm_stream_si128, _mm_load_si128 )
		STEP(023, _mm_stream_si128, _mm_load_si128 )
		STEP(022, _mm_stream_si128, _mm_load_si128 )
		STEP(021, _mm_stream_si128, _mm_load_si128 )
		STEP(020, _mm_stream_si128, _mm_load_si128 )
			} while(_MCFCRT_EXPECT(rem != 0));
			_mm_sfence();
//=============================================================================
			break;
		// non-temporal, unaligned read
			do {
		STEP(037, _mm_stream_si128, _mm_loadu_si128)
		STEP(036, _mm_stream_si128, _mm_loadu_si128)
		STEP(035, _mm_stream_si128, _mm_loadu_si128)
		STEP(034, _mm_stream_si128, _mm_loadu_si128)
		STEP(033, _mm_stream_si128, _mm_loadu_si128)
		STEP(032, _mm_stream_si128, _mm_loadu_si128)
		STEP(031, _mm_stream_si128, _mm_loadu_si128)
		STEP(030, _mm_stream_si128, _mm_loadu_si128)
			} while(_MCFCRT_EXPECT(rem != 0));
			_mm_sfence();
//=============================================================================
#undef STEP
		}
	}
	rem = (n - off) % 16;
#define STEP(b_)	\
	if(_MCFCRT_EXPECT(rem & (b_ / 8))){	\
		*(volatile uint##b_##_t *)wp = *(uint##b_##_t *)rp;	\
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

	unsigned char *wp = (unsigned char *)s1;
	const unsigned char *rp = (const unsigned char *)s2;
	const size_t off = (uintptr_t)s1 % 16; // off = misalignment
	__m128i mis_w;
	if(_MCFCRT_EXPECT(off != 0)){
		mis_w = _mm_loadu_si128((const __m128i *)s2 - 1);
		wp -= off;
		rp -= off;
	}
	size_t rem = (n - off) / 16;
	if(_MCFCRT_EXPECT(rem != 0)){
		const size_t nt = !!(n >> 20) << 4;
		const size_t ur = !!((uintptr_t)rp % 16) << 3;
		switch((rem - 1) % 8 + nt + ur){
#define STEP(k_, store_, load_)	\
				__attribute__((__fallthrough__));	\
		case (k_):	\
				wp -= 16;	\
				rp -= 16;	\
				store_((__m128i *)wp, load_((const __m128i *)rp));	\
				--rem;
//=============================================================================
			break;
		// temporal, aligned read
			do {
		STEP(007, _mm_store_si128 , _mm_load_si128 )
		STEP(006, _mm_store_si128 , _mm_load_si128 )
		STEP(005, _mm_store_si128 , _mm_load_si128 )
		STEP(004, _mm_store_si128 , _mm_load_si128 )
		STEP(003, _mm_store_si128 , _mm_load_si128 )
		STEP(002, _mm_store_si128 , _mm_load_si128 )
		STEP(001, _mm_store_si128 , _mm_load_si128 )
		STEP(000, _mm_store_si128 , _mm_load_si128 )
			} while(_MCFCRT_EXPECT(rem != 0));
//=============================================================================
			break;
		// temporal, unaligned read
			do {
		STEP(017, _mm_store_si128 , _mm_loadu_si128)
		STEP(016, _mm_store_si128 , _mm_loadu_si128)
		STEP(015, _mm_store_si128 , _mm_loadu_si128)
		STEP(014, _mm_store_si128 , _mm_loadu_si128)
		STEP(013, _mm_store_si128 , _mm_loadu_si128)
		STEP(012, _mm_store_si128 , _mm_loadu_si128)
		STEP(011, _mm_store_si128 , _mm_loadu_si128)
		STEP(010, _mm_store_si128 , _mm_loadu_si128)
			} while(_MCFCRT_EXPECT(rem != 0));
//=============================================================================
			break;
		// non-temporal, aligned read
			do {
		STEP(027, _mm_stream_si128, _mm_load_si128 )
		STEP(026, _mm_stream_si128, _mm_load_si128 )
		STEP(025, _mm_stream_si128, _mm_load_si128 )
		STEP(024, _mm_stream_si128, _mm_load_si128 )
		STEP(023, _mm_stream_si128, _mm_load_si128 )
		STEP(022, _mm_stream_si128, _mm_load_si128 )
		STEP(021, _mm_stream_si128, _mm_load_si128 )
		STEP(020, _mm_stream_si128, _mm_load_si128 )
			} while(_MCFCRT_EXPECT(rem != 0));
			_mm_sfence();
//=============================================================================
			break;
		// non-temporal, unaligned read
			do {
		STEP(037, _mm_stream_si128, _mm_loadu_si128)
		STEP(036, _mm_stream_si128, _mm_loadu_si128)
		STEP(035, _mm_stream_si128, _mm_loadu_si128)
		STEP(034, _mm_stream_si128, _mm_loadu_si128)
		STEP(033, _mm_stream_si128, _mm_loadu_si128)
		STEP(032, _mm_stream_si128, _mm_loadu_si128)
		STEP(031, _mm_stream_si128, _mm_loadu_si128)
		STEP(030, _mm_stream_si128, _mm_loadu_si128)
			} while(_MCFCRT_EXPECT(rem != 0));
			_mm_sfence();
//=============================================================================
#undef STEP
		}
	}
	rem = (n - off) % 16;
#define STEP(b_)	\
	if(_MCFCRT_EXPECT(rem & (b_ / 8))){	\
		wp -= b_ / 8;	\
		rp -= b_ / 8;	\
		*(volatile uint##b_##_t *)wp = *(uint##b_##_t *)rp;	\
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
