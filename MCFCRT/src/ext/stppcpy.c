// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "stppcpy.h"
#include "../env/expect.h"
#include "../env/xassert.h"
#include "../stdc/string/_sse3.h"
#include "rep_movs.h"

char *_MCFCRT_stppcpy(char *s1, char *es1, const char *restrict s2){
	_MCFCRT_ASSERT(s1 < es1);

	// 如果 rp 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	register const char *rp = (const char *)((uintptr_t)s2 & (uintptr_t)-32);
	char *ewp = s1;
	const char *erp = s2;
	__m128i xz[1];
	__MCFCRT_xmmsetz(xz);
	unsigned shift = (unsigned)((const char *)s2 - rp);
	uint32_t skip = (uint32_t)-1 << shift;
//=============================================================================
#define LOOP_BODY(ewp_next_)	\
	{	\
		__m128i xw[2];	\
		uint32_t mask;	\
		rp = __MCFCRT_xmmload_2(xw, rp, _mm_load_si128);	\
		mask = __MCFCRT_xmmcmp_21b(xw, xz, _mm_cmpeq_epi8) & skip;	\
		ptrdiff_t dist = rp - (s2 + (es1 - s1) - 1);	\
		dist &= ~dist >> (sizeof(dist) * 8 - 1);	\
		mask |= ~((uint32_t)-1 >> dist);	\
		__builtin_prefetch(rp + 64, 0, 0);	\
		if(_MCFCRT_EXPECT_NOT(mask != 0)){	\
			shift = (unsigned)__builtin_ctzl(mask);	\
			ewp = _MCFCRT_rep_movsb(ewp, erp, (size_t)(rp - 32 + shift - erp));	\
			*ewp = 0;	\
			return ewp;	\
		}	\
		ewp = (ewp_next_);	\
		erp = rp;	\
		skip = (uint32_t)-1;	\
	}
//=============================================================================
	LOOP_BODY(_MCFCRT_rep_movsb(ewp, erp, (size_t)(rp - erp)));
	if(((uintptr_t)ewp & ~(uintptr_t)-16) == 0){
		for(;;){
			LOOP_BODY(__MCFCRT_xmmstore_2(ewp, xw, _mm_store_si128));
		}
	} else {
		for(;;){
			LOOP_BODY(__MCFCRT_xmmstore_2(ewp, xw, _mm_storeu_si128));
		}
	}
}
