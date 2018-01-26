// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#define __MCFCRT_MEMCPY_IMPL_INLINE_OR_EXTERN     extern inline
#include "_memcpy_impl.h"

#pragma GCC diagnostic ignored "-Wswitch-unreachable"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"

static inline void __MCFCRT_memcpy_aligned32_fwd(unsigned char **_MCFCRT_RESTRICT __wp, const unsigned char **_MCFCRT_RESTRICT __rp) _MCFCRT_NOEXCEPT {
	// TODO: Rewrite to make use of AVX in the future.
	_mm_store_ps((float *)*__wp, _mm_loadu_ps((const float *)*__rp));
	*__wp += 16;
	*__rp += 16;
	_mm_store_ps((float *)*__wp, _mm_loadu_ps((const float *)*__rp));
	*__wp += 16;
	*__rp += 16;
}
static inline void __MCFCRT_memcpy_aligned32_bwd(unsigned char **_MCFCRT_RESTRICT __wp, const unsigned char **_MCFCRT_RESTRICT __rp) _MCFCRT_NOEXCEPT {
	// TODO: Rewrite to make use of AVX in the future.
	*__wp -= 16;
	*__rp -= 16;
	_mm_store_ps((float *)*__wp, _mm_loadu_ps((const float *)*__rp));
	*__wp -= 16;
	*__rp -= 16;
	_mm_store_ps((float *)*__wp, _mm_loadu_ps((const float *)*__rp));
}

static inline void __MCFCRT_memcpy_nontemp32_fwd(unsigned char **_MCFCRT_RESTRICT __wp, const unsigned char **_MCFCRT_RESTRICT __rp) _MCFCRT_NOEXCEPT {
	// TODO: Rewrite to make use of AVX in the future.
	_mm_stream_ps((float *)*__wp, _mm_loadu_ps((const float *)*__rp));
	*__wp += 16;
	*__rp += 16;
	_mm_stream_ps((float *)*__wp, _mm_loadu_ps((const float *)*__rp));
	*__wp += 16;
	*__rp += 16;
}
static inline void __MCFCRT_memcpy_nontemp32_bwd(unsigned char **_MCFCRT_RESTRICT __wp, const unsigned char **_MCFCRT_RESTRICT __rp) _MCFCRT_NOEXCEPT {
	// TODO: Rewrite to make use of AVX in the future.
	*__wp -= 16;
	*__rp -= 16;
	_mm_stream_ps((float *)*__wp, _mm_loadu_ps((const float *)*__rp));
	*__wp -= 16;
	*__rp -= 16;
	_mm_stream_ps((float *)*__wp, _mm_loadu_ps((const float *)*__rp));
}

void __MCFCRT_memcpy_large_fwd(unsigned char *bwp, unsigned char *ewp, const unsigned char *brp, const unsigned char *erp){
	_MCFCRT_ASSERT(ewp - bwp == erp - brp);
	_MCFCRT_ASSERT(ewp - bwp >= 64);
	unsigned char *wp = bwp;
	const unsigned char *rp = brp;
	// Copy the initial, potentially unaligned word.
	__MCFCRT_memcpy_piece32_fwd(&wp, &rp);
	// If there is misalignment at all, align the write pointer to 32-byte boundaries, rounding downwards.
	wp = (unsigned char *)((uintptr_t)wp & (uintptr_t)-16);
	rp = erp - (ewp - wp);
	// Copy words to aligned locations.
	if(_MCFCRT_EXPECT((size_t)(ewp - wp) < 0x100000)){
		// We will copy the final word separately, hence the last (0,32] bytes are excluded here.
		switch((size_t)(ewp - wp - 1) / 32 % 16){
			do {
#define COPY_WORD(k_)	\
			__MCFCRT_memcpy_aligned32_fwd(&wp, &rp);	\
		case (k_):	\
			;
//=============================================================================
		COPY_WORD(017)  COPY_WORD(016)  COPY_WORD(015)  COPY_WORD(014)
		COPY_WORD(013)  COPY_WORD(012)  COPY_WORD(011)  COPY_WORD(010)
		COPY_WORD(007)  COPY_WORD(006)  COPY_WORD(005)  COPY_WORD(004)
		COPY_WORD(003)  COPY_WORD(002)  COPY_WORD(001)  COPY_WORD(000)
//=============================================================================
#undef COPY_WORD
			} while(_MCFCRT_EXPECT((size_t)(ewp - wp) > 32));
		}
	} else {
		// See comments in the other branch.
		switch((size_t)(ewp - wp - 1) / 32 % 16){
			do {
#define COPY_WORD(k_)	\
			__MCFCRT_memcpy_nontemp32_fwd(&wp, &rp);	\
		case (k_):	\
			;
//=============================================================================
		COPY_WORD(017)  COPY_WORD(016)  COPY_WORD(015)  COPY_WORD(014)
		COPY_WORD(013)  COPY_WORD(012)  COPY_WORD(011)  COPY_WORD(010)
		COPY_WORD(007)  COPY_WORD(006)  COPY_WORD(005)  COPY_WORD(004)
		COPY_WORD(003)  COPY_WORD(002)  COPY_WORD(001)  COPY_WORD(000)
//=============================================================================
#undef COPY_WORD
			} while(_MCFCRT_EXPECT((size_t)(ewp - wp) > 32));
		}
		// Don't forget to emit a store fence.
		_mm_sfence();
	}
	// Copy the final, potentially unaligned word.
	wp = ewp;
	rp = erp;
	__MCFCRT_memcpy_piece32_bwd(&wp, &rp);
}
void __MCFCRT_memcpy_large_bwd(unsigned char *bwp, unsigned char *ewp, const unsigned char *brp, const unsigned char *erp){
	_MCFCRT_ASSERT(ewp - bwp == erp - brp);
	_MCFCRT_ASSERT(ewp - bwp >= 64);
	unsigned char *wp = ewp;
	const unsigned char *rp = erp;
	// Copy the final, potentially unaligned word.
	__MCFCRT_memcpy_piece32_bwd(&wp, &rp);
	// If there is misalignment at all, align the write pointer to 32-byte boundaries, rounding downwards.
	wp = (unsigned char *)((uintptr_t)(wp + 15) & (uintptr_t)-16);
	rp = brp + (wp - bwp);
	// Copy words to aligned locations.
	if(_MCFCRT_EXPECT((size_t)(wp - bwp) < 0x100000)){
		// We will copy the initial word separately, hence the last (0,32] bytes are excluded here.
		switch((size_t)(wp - bwp - 1) / 32 % 16){
			do {
#define COPY_WORD(k_)	\
			__MCFCRT_memcpy_aligned32_bwd(&wp, &rp);	\
		case (k_):	\
			;
//=============================================================================
		COPY_WORD(017)  COPY_WORD(016)  COPY_WORD(015)  COPY_WORD(014)
		COPY_WORD(013)  COPY_WORD(012)  COPY_WORD(011)  COPY_WORD(010)
		COPY_WORD(007)  COPY_WORD(006)  COPY_WORD(005)  COPY_WORD(004)
		COPY_WORD(003)  COPY_WORD(002)  COPY_WORD(001)  COPY_WORD(000)
//=============================================================================
#undef COPY_WORD
			} while(_MCFCRT_EXPECT((size_t)(wp - bwp) > 32));
		}
	} else {
		// See comments in the other branch.
		switch((size_t)(wp - bwp - 1) / 32 % 16){
			do {
#define COPY_WORD(k_)	\
			__MCFCRT_memcpy_nontemp32_bwd(&wp, &rp);	\
		case (k_):	\
			;
//=============================================================================
		COPY_WORD(017)  COPY_WORD(016)  COPY_WORD(015)  COPY_WORD(014)
		COPY_WORD(013)  COPY_WORD(012)  COPY_WORD(011)  COPY_WORD(010)
		COPY_WORD(007)  COPY_WORD(006)  COPY_WORD(005)  COPY_WORD(004)
		COPY_WORD(003)  COPY_WORD(002)  COPY_WORD(001)  COPY_WORD(000)
//=============================================================================
#undef COPY_WORD
			} while(_MCFCRT_EXPECT((size_t)(wp - bwp) > 32));
		}
		// Don't forget to emit a store fence.
		_mm_sfence();
	}
	// Copy the initial, potentially unaligned word.
	wp = bwp;
	rp = brp;
	__MCFCRT_memcpy_piece32_fwd(&wp, &rp);
}
