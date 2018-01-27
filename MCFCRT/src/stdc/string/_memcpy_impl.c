// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#define __MCFCRT_MEMCPY_IMPL_INLINE_OR_EXTERN     extern inline
#include "_memcpy_impl.h"

#pragma GCC diagnostic ignored "-Wswitch-unreachable"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"

void __MCFCRT_memcpy_large_fwd(unsigned char *bwp, unsigned char *ewp, const unsigned char *brp, const unsigned char *erp){
	_MCFCRT_ASSERT(ewp - bwp == erp - brp);
	_MCFCRT_ASSERT(ewp - bwp >= 64);
	unsigned char *wp = bwp;
	const unsigned char *rp = brp;
	// Copy the initial, potentially unaligned QQWORD.
	__MCFCRT_memcpy_piece32_fwd(&wp, &rp);
	// If there is misalignment at all, align the write pointer to 32-byte boundaries, rounding downwards.
	wp = (unsigned char *)((uintptr_t)wp & (uintptr_t)-16);
	rp = erp - (ewp - wp);
	// Copy QQWORDs to aligned locations.
	// We will copy the final QQWORD separately, hence the last (0,32] bytes are excluded here.
	switch((size_t)(ewp - wp - 1) / 32 % 16){
		do {
		// TODO: Rewrite to make use of AVX in the future.
#define COPY_QQWORD(k_)	\
		_mm_store_ps((float *)wp, _mm_loadu_ps((const float *)rp));	\
		wp += 16;	\
		rp += 16;	\
		_mm_store_ps((float *)wp, _mm_loadu_ps((const float *)rp));	\
		wp += 16;	\
		rp += 16;	\
	case (k_):	\
		;
//=============================================================================
	COPY_QQWORD(017)  COPY_QQWORD(016)  COPY_QQWORD(015)  COPY_QQWORD(014)
	COPY_QQWORD(013)  COPY_QQWORD(012)  COPY_QQWORD(011)  COPY_QQWORD(010)
	COPY_QQWORD(007)  COPY_QQWORD(006)  COPY_QQWORD(005)  COPY_QQWORD(004)
	COPY_QQWORD(003)  COPY_QQWORD(002)  COPY_QQWORD(001)  COPY_QQWORD(000)
//=============================================================================
#undef COPY_QQWORD
		} while(_MCFCRT_EXPECT((size_t)(ewp - wp) > 32));
	}
	// Copy the final, potentially unaligned QQWORD.
	wp = ewp;
	rp = erp;
	__MCFCRT_memcpy_piece32_bwd(&wp, &rp);
}
void __MCFCRT_memcpy_large_bwd(unsigned char *bwp, unsigned char *ewp, const unsigned char *brp, const unsigned char *erp){
	_MCFCRT_ASSERT(ewp - bwp == erp - brp);
	_MCFCRT_ASSERT(ewp - bwp >= 64);
	unsigned char *wp = ewp;
	const unsigned char *rp = erp;
	// Copy the final, potentially unaligned QQWORD.
	__MCFCRT_memcpy_piece32_bwd(&wp, &rp);
	// If there is misalignment at all, align the write pointer to 32-byte boundaries, rounding downwards.
	wp = (unsigned char *)((uintptr_t)(wp + 15) & (uintptr_t)-16);
	rp = brp + (wp - bwp);
	// Copy QQWORDs to aligned locations.
	// We will copy the initial QQWORD separately, hence the last (0,32] bytes are excluded here.
	switch((size_t)(wp - bwp - 1) / 32 % 16){
		do {
		// TODO: Rewrite to make use of AVX in the future.
#define COPY_QQWORD(k_)	\
		wp -= 16;	\
		rp -= 16;	\
		_mm_store_ps((float *)wp, _mm_loadu_ps((const float *)rp));	\
		wp -= 16;	\
		rp -= 16;	\
		_mm_store_ps((float *)wp, _mm_loadu_ps((const float *)rp));	\
	case (k_):	\
		;
//=============================================================================
	COPY_QQWORD(017)  COPY_QQWORD(016)  COPY_QQWORD(015)  COPY_QQWORD(014)
	COPY_QQWORD(013)  COPY_QQWORD(012)  COPY_QQWORD(011)  COPY_QQWORD(010)
	COPY_QQWORD(007)  COPY_QQWORD(006)  COPY_QQWORD(005)  COPY_QQWORD(004)
	COPY_QQWORD(003)  COPY_QQWORD(002)  COPY_QQWORD(001)  COPY_QQWORD(000)
//=============================================================================
#undef COPY_QQWORD
		} while(_MCFCRT_EXPECT((size_t)(wp - bwp) > 32));
	}
	// Copy the initial, potentially unaligned QQWORD.
	wp = bwp;
	rp = brp;
	__MCFCRT_memcpy_piece32_fwd(&wp, &rp);
}

void __MCFCRT_memcpy_huge_fwd(unsigned char *bwp, unsigned char *ewp, const unsigned char *brp, const unsigned char *erp){
	_MCFCRT_ASSERT(ewp - bwp == erp - brp);
	_MCFCRT_ASSERT(ewp - bwp >= 64);
	unsigned char *wp = bwp;
	const unsigned char *rp = brp;
	// Copy the initial, potentially unaligned QQWORD.
	__MCFCRT_memcpy_piece32_fwd(&wp, &rp);
	// If there is misalignment at all, align the write pointer to 32-byte boundaries, rounding downwards.
	wp = (unsigned char *)((uintptr_t)wp & (uintptr_t)-16);
	rp = erp - (ewp - wp);
	// Copy QQWORDs to aligned locations.
	// We will copy the final QQWORD separately, hence the last (0,32] bytes are excluded here.
	switch((size_t)(ewp - wp - 1) / 32 % 16){
		do {
		// TODO: Rewrite to make use of AVX in the future.
#define COPY_QQWORD(k_)	\
		_mm_stream_ps((float *)wp, _mm_loadu_ps((const float *)rp));	\
		wp += 16;	\
		rp += 16;	\
		_mm_stream_ps((float *)wp, _mm_loadu_ps((const float *)rp));	\
		wp += 16;	\
		rp += 16;	\
	case (k_):	\
		;
//=============================================================================
	COPY_QQWORD(017)  COPY_QQWORD(016)  COPY_QQWORD(015)  COPY_QQWORD(014)
	COPY_QQWORD(013)  COPY_QQWORD(012)  COPY_QQWORD(011)  COPY_QQWORD(010)
	COPY_QQWORD(007)  COPY_QQWORD(006)  COPY_QQWORD(005)  COPY_QQWORD(004)
	COPY_QQWORD(003)  COPY_QQWORD(002)  COPY_QQWORD(001)  COPY_QQWORD(000)
//=============================================================================
#undef COPY_QQWORD
		} while(_MCFCRT_EXPECT((size_t)(ewp - wp) > 32));
	}
	// Don't forget to emit a store fence.
	_mm_sfence();
	// Copy the final, potentially unaligned QQWORD.
	wp = ewp;
	rp = erp;
	__MCFCRT_memcpy_piece32_bwd(&wp, &rp);
}
void __MCFCRT_memcpy_huge_bwd(unsigned char *bwp, unsigned char *ewp, const unsigned char *brp, const unsigned char *erp){
	_MCFCRT_ASSERT(ewp - bwp == erp - brp);
	_MCFCRT_ASSERT(ewp - bwp >= 64);
	unsigned char *wp = ewp;
	const unsigned char *rp = erp;
	// Copy the final, potentially unaligned QQWORD.
	__MCFCRT_memcpy_piece32_bwd(&wp, &rp);
	// If there is misalignment at all, align the write pointer to 32-byte boundaries, rounding downwards.
	wp = (unsigned char *)((uintptr_t)(wp + 15) & (uintptr_t)-16);
	rp = brp + (wp - bwp);
	// Copy QQWORDs to aligned locations.
	// We will copy the initial QQWORD separately, hence the last (0,32] bytes are excluded here.
	switch((size_t)(wp - bwp - 1) / 32 % 16){
		do {
		// TODO: Rewrite to make use of AVX in the future.
#define COPY_QQWORD(k_)	\
		wp -= 16;	\
		rp -= 16;	\
		_mm_stream_ps((float *)wp, _mm_loadu_ps((const float *)rp));	\
		wp -= 16;	\
		rp -= 16;	\
		_mm_stream_ps((float *)wp, _mm_loadu_ps((const float *)rp));	\
	case (k_):	\
		;
//=============================================================================
	COPY_QQWORD(017)  COPY_QQWORD(016)  COPY_QQWORD(015)  COPY_QQWORD(014)
	COPY_QQWORD(013)  COPY_QQWORD(012)  COPY_QQWORD(011)  COPY_QQWORD(010)
	COPY_QQWORD(007)  COPY_QQWORD(006)  COPY_QQWORD(005)  COPY_QQWORD(004)
	COPY_QQWORD(003)  COPY_QQWORD(002)  COPY_QQWORD(001)  COPY_QQWORD(000)
//=============================================================================
#undef COPY_QQWORD
		} while(_MCFCRT_EXPECT((size_t)(wp - bwp) > 32));
	}
	// Don't forget to emit a store fence.
	_mm_sfence();
	// Copy the initial, potentially unaligned QQWORD.
	wp = bwp;
	rp = brp;
	__MCFCRT_memcpy_piece32_fwd(&wp, &rp);
}
