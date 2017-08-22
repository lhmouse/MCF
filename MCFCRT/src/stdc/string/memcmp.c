// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../env/expect.h"
#include "_sse2.h"
#include "_ssse3.h"

#undef memcmp

int memcmp(const void *s1, const void *s2, size_t n){
	// 如果 arp1 和 arp2 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	register const unsigned char *arp1 = (const unsigned char *)((uintptr_t)s1 & (uintptr_t)-32);
	register const unsigned char *arp2 = (const unsigned char *)((uintptr_t)s2 & (uintptr_t)-32);
	const unsigned align = (unsigned)(32 - ((const unsigned char *)s1 - arp1) + ((const unsigned char *)s2 - arp2));

	__m128i s2v[6];
	bool s2z;
	__m128i xw[2], xc[2];
	uint32_t mask;
	ptrdiff_t dist;
//=============================================================================
#define BEGIN	\
	if(_MCFCRT_EXPECT_NOT(arp1 >= (const unsigned char *)s1 + n)){	\
		goto end_equal;	\
	}	\
	arp1 = __MCFCRT_xmmload_2(xw, arp1, _mm_load_si128);
#define NEXT	\
	for(unsigned i = 0; i < 4; ++i){	\
		s2v[i] = s2v[i + 2];	\
	}	\
	if(_MCFCRT_EXPECT(!s2z)){	\
		arp2 = __MCFCRT_xmmload_2(s2v + 4, arp2, _mm_load_si128);	\
		s2z = arp2 >= (const unsigned char *)s2 + n;	\
	}	\
	__MCFCRT_xmmalign_26(xc, s2v, align);	\
	mask = ~__MCFCRT_xmmcmp_22b(xw, xc, _mm_cmpeq_epi8);
#define END	\
	dist = arp1 - ((const unsigned char *)s1 + n);	\
	dist &= ~dist >> (sizeof(dist) * 8 - 1);	\
	mask |= ~((uint32_t)-1 >> dist);	\
	_mm_prefetch(arp1 + 256, _MM_HINT_T1);	\
	_mm_prefetch(arp2 + 256, _MM_HINT_T1);	\
	if(_MCFCRT_EXPECT_NOT(mask != 0)){	\
		goto end;	\
	}
//=============================================================================
	BEGIN
	__MCFCRT_xmmsetz_2(s2v + 2);
	arp2 = __MCFCRT_xmmload_2(s2v + 4, arp2, _mm_load_si128);
	s2z = arp2 >= (const unsigned char *)s2 + n;
	NEXT
	mask &= (uint32_t)-1 << (((const unsigned char *)s1 - arp1) & 0x1F);
	END
	for(;;){
		BEGIN
		NEXT
		END
	}
end:
	if((mask << dist) != 0){
		arp1 = arp1 - 32 + (unsigned)__builtin_ctzl(mask);
		arp2 = arp1 - (const unsigned char *)s1 + (const unsigned char *)s2;
		return (*arp1 < *arp2) ? -1 : 1;
	}
end_equal:
	return 0;
}


/*
	__m128i axv[3][2];
	__MCFCRT_xmmsetz_2(axv[0]);
	bool s2z;
//=============================================================================
#define LOOP_BODY(skip_, xva_, xvb_, xvc_, align_, ...)	\
	{	\
		if(_MCFCRT_EXPECT_NOT(arp1 >= (const unsigned char *)s1 + n)){	\
			return 0;	\
		}	\
		__m128i xw[2];	\
		__m128i xc[2];	\
		uint32_t mask;	\
		ptrdiff_t dist;	\
		arp1 = __MCFCRT_xmmload_2(xw, arp1, _mm_load_si128);	\
		{ __VA_ARGS__ };	\
		if(_MCFCRT_EXPECT(!s2z)){	\
			arp2 = __MCFCRT_xmmload_2((xvc_), arp2, _mm_load_si128);	\
			s2z = arp2 < (const unsigned char *)s2 + n;	\
		}	\
		switch((align_) / 16){	\
		case 0:	\
			xc[0] = _mm_alignr_epi8((xva_)[1], (xva_)[0], (align_) % 16);	\
			xc[1] = _mm_alignr_epi8((xvb_)[0], (xva_)[1], (align_) % 16);	\
			break;	\
		case 1:	\
			xc[0] = _mm_alignr_epi8((xvb_)[0], (xva_)[1], (align_) % 16);	\
			xc[1] = _mm_alignr_epi8((xvb_)[1], (xvb_)[0], (align_) % 16);	\
			break;	\
		case 2:	\
			xc[0] = _mm_alignr_epi8((xvb_)[1], (xvb_)[0], (align_) % 16);	\
			xc[1] = _mm_alignr_epi8((xvc_)[0], (xvb_)[1], (align_) % 16);	\
			break;	\
		case 3:	\
			xc[0] = _mm_alignr_epi8((xvc_)[0], (xvb_)[1], (align_) % 16);	\
			xc[1] = _mm_alignr_epi8((xvc_)[1], (xvc_)[0], (align_) % 16);	\
			break;	\
		default:	\
			__builtin_trap();	\
		}	\
		mask = ~__MCFCRT_xmmcmp_22b(xw, xc, _mm_cmpeq_epi8);	\
		mask &= (skip_);	\
		dist = arp1 - ((const unsigned char *)s1 + n);	\
		dist &= ~dist >> (sizeof(dist) * 8 - 1);	\
		mask |= ~((uint32_t)-1 >> dist);	\
		_mm_prefetch(arp1 + 256, _MM_HINT_T1);	\
		_mm_prefetch(arp2 + 256, _MM_HINT_T1);	\
		if(_MCFCRT_EXPECT_NOT(mask != 0)){	\
			if((mask << dist) == 0){	\
				return 0;	\
			}	\
			const unsigned char *const rp1 = arp1 - 32 + (unsigned)__builtin_ctzl(mask);	\
			const unsigned char *const rp2 = (const unsigned char *)s2 + (rp1 - (const unsigned char *)s1);	\
			return (*rp1 < *rp2) ? -1 : 1;	\
		}	\
	}
//=============================================================================
	switch((unsigned)(32 + (arp1 - (const unsigned char *)s1) - (arp2 - (const unsigned char *)s2))){
#define CASE(k_)	\
	case (k_):	\
		LOOP_BODY((uint32_t)-1 << ((const unsigned char *)s1 - arp1), axv[0], axv[1], axv[2], (k_),	\
			arp2 = __MCFCRT_xmmload_2(axv[1], arp2, _mm_load_si128);	\
			s2z = arp2 < (const unsigned char *)s2 + n;	\
		)	\
		for(;;){	\
			LOOP_BODY((uint32_t)-1, axv[1], axv[2], axv[0], (k_), )	\
			LOOP_BODY((uint32_t)-1, axv[2], axv[0], axv[1], (k_), )	\
			LOOP_BODY((uint32_t)-1, axv[0], axv[1], axv[2], (k_), )	\
		}
//-----------------------------------------------------------------------------
	           CASE(001)  CASE(002)  CASE(003)  CASE(004)  CASE(005)  CASE(006)  CASE(007)
	CASE(010)  CASE(011)  CASE(012)  CASE(013)  CASE(014)  CASE(015)  CASE(016)  CASE(017)
	CASE(020)  CASE(021)  CASE(022)  CASE(023)  CASE(024)  CASE(025)  CASE(026)  CASE(027)
	CASE(030)  CASE(031)  CASE(032)  CASE(033)  CASE(034)  CASE(035)  CASE(036)  CASE(037)
	CASE(040)  CASE(041)  CASE(042)  CASE(043)  CASE(044)  CASE(045)  CASE(046)  CASE(047)
	CASE(050)  CASE(051)  CASE(052)  CASE(053)  CASE(054)  CASE(055)  CASE(056)  CASE(057)
	CASE(060)  CASE(061)  CASE(062)  CASE(063)  CASE(064)  CASE(065)  CASE(066)  CASE(067)
	CASE(070)  CASE(071)  CASE(072)  CASE(073)  CASE(074)  CASE(075)  CASE(076)  CASE(077)
//-----------------------------------------------------------------------------
#undef CASE
	default:
		__builtin_trap();
	}
}
*/
/*
#undef memcmp

int memcmp(const void *s1, const void *s2, size_t n){
	register const char *rp1 = s1;
	const char *const rend1 = rp1 + n;
	register const char *rp2 = s2;
	// 如果 rp 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	while(((uintptr_t)rp1 & 31) != 0){
#define CMP_GEN()	\
		{	\
			if(rp1 == rend1){	\
				return 0;	\
			}	\
			const unsigned c1 = (uint8_t)*rp1;	\
			const unsigned c2 = (uint8_t)*rp2;	\
			if(c1 != c2){	\
				return (c1 < c2) ? -1 : 1;	\
			}	\
			++rp1;	\
			++rp2;	\
		}
		CMP_GEN()
	}
	if((size_t)(rend1 - rp1) >= 64){
#define CMP_SSE3(load2_)	\
		{	\
			do {	\
				const __m128i xw10 = _mm_load_si128((const __m128i *)rp1 + 0);	\
				const __m128i xw11 = _mm_load_si128((const __m128i *)rp1 + 1);	\
				const __m128i xw20 = load2_((const __m128i *)rp2 + 0);	\
				const __m128i xw21 = load2_((const __m128i *)rp2 + 1);	\
				__m128i xt = _mm_cmpeq_epi8(xw10, xw20);	\
				uint32_t mask = (uint32_t)_mm_movemask_epi8(xt) ^ 0xFFFF;	\
				xt = _mm_cmpeq_epi8(xw11, xw21);	\
				mask += ((uint32_t)_mm_movemask_epi8(xt) ^ 0xFFFF) << 16;	\
				if(_MCFCRT_EXPECT_NOT(mask != 0)){	\
					const int tzne = __builtin_ctzl(mask);	\
					const __m128i shift = _mm_set1_epi8(-0x80);	\
					xt = _mm_cmpgt_epi8(_mm_add_epi8(xw10, shift),	\
					                    _mm_add_epi8(xw20, shift));	\
					mask = (uint32_t)_mm_movemask_epi8(xt);	\
					xt = _mm_cmpgt_epi8(_mm_add_epi8(xw11, shift),	\
					                    _mm_add_epi8(xw21, shift));	\
					mask += (uint32_t)_mm_movemask_epi8(xt) << 16;	\
					const int tzgt = (mask == 0) ? 32 : __builtin_ctzl(mask);	\
					return ((tzne - tzgt) >> 15) | 1;	\
				}	\
				rp1 += 32;	\
				rp2 += 32;	\
			} while((size_t)(rend1 - rp1) >= 32);	\
		}
		if(((uintptr_t)rp2 & 15) == 0){
			CMP_SSE3(_mm_load_si128)
		} else {
			CMP_SSE3(_mm_lddqu_si128)
		}
	}
	for(;;){
		CMP_GEN()
	}
}


#undef memchr

void *memchr(const void *s, int c, size_t n){
	// 如果 arp 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	register const unsigned char *arp = (const unsigned char *)((uintptr_t)s & (uintptr_t)-32);
	__m128i xc[1];
	__MCFCRT_xmmsetb(xc, (uint8_t)c);

	__m128i xw[2];
	uint32_t mask;
	ptrdiff_t dist;
//=============================================================================
#define BEGIN	\
	if(_MCFCRT_EXPECT_NOT(arp >= (const unsigned char *)s + n)){	\
		goto end_null;	\
	}	\
	arp = __MCFCRT_xmmload_2(xw, arp, _mm_load_si128);	\
	mask = __MCFCRT_xmmcmp_21b(xw, xc, _mm_cmpeq_epi8);
#define END	\
	dist = arp - ((const unsigned char *)s + n);	\
	dist &= ~dist >> (sizeof(dist) * 8 - 1);	\
	mask |= ~((uint32_t)-1 >> dist);	\
	_mm_prefetch(arp + 256, _MM_HINT_T1);	\
	if(_MCFCRT_EXPECT_NOT(mask != 0)){	\
		goto end;	\
	}
//=============================================================================
	BEGIN
	mask &= (uint32_t)-1 << ((const unsigned char *)s - arp);
	END
	for(;;){
		BEGIN
		END
	}
end:
	if((mask << dist) != 0){
		arp = arp - 32 + (unsigned)__builtin_ctzl(mask);
		return (unsigned char *)arp;
	}
end_null:
	return _MCFCRT_NULLPTR;
}

*/
