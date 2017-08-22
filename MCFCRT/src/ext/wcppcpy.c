// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "wcppcpy.h"
#include "../env/expect.h"
#include "../env/xassert.h"
#include "../stdc/string/_sse2.h"
#include "rep_movs.h"

wchar_t *_MCFCRT_wcppcpy(wchar_t *s1, wchar_t *es1, const wchar_t *restrict s2){
	_MCFCRT_ASSERT(s1 < es1);

	// 如果 arp 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	register wchar_t *wp = s1;
	register const wchar_t *arp = (const wchar_t *)((uintptr_t)s2 & (uintptr_t)-64);
	__m128i xz[1];
	__MCFCRT_xmmsetz(xz);

	__m128i xw[4];
	uint32_t mask;
	ptrdiff_t dist;
//=============================================================================
#define BEGIN	\
	arp = __MCFCRT_xmmload_4(xw, arp, _mm_load_si128);	\
	mask = __MCFCRT_xmmcmp_41w(xw, xz, _mm_cmpeq_epi16);
#define BREAK_OPEN	\
	dist = arp - (s2 + (es1 - s1) - 1);	\
	dist &= ~dist >> (sizeof(dist) * 8 - 1);	\
	mask |= ~((uint32_t)-1 >> dist);	\
	_mm_prefetch(arp + 256, _MM_HINT_T1);	\
	if(_MCFCRT_EXPECT_NOT(mask != 0)){
#define BREAK_CLOSE	\
		goto end;	\
	}
#define END	\
	//
//=============================================================================
	BEGIN
	mask &= (uint32_t)-1 << (((const wchar_t *)s2 - arp) & 0x1F);
	BREAK_OPEN
	wp = (wchar_t *)_MCFCRT_rep_movsw(_MCFCRT_NULLPTR, (uint16_t *)wp, (const uint16_t *)s2, (size_t)(arp - 32 + (unsigned)__builtin_ctzl(mask) - s2));
	BREAK_CLOSE
	wp = (wchar_t *)_MCFCRT_rep_movsw(_MCFCRT_NULLPTR, (uint16_t *)wp, (const uint16_t *)s2, (size_t)(arp - s2));
	END
	if(((uintptr_t)wp & ~(uintptr_t)-16) == 0){
		for(;;){
			BEGIN
			BREAK_OPEN
			wp = (wchar_t *)_MCFCRT_rep_movsw(_MCFCRT_NULLPTR, (uint16_t *)wp, (const uint16_t *)arp - 32, (unsigned)__builtin_ctzl(mask));
			BREAK_CLOSE
			wp = __MCFCRT_xmmstore_4(wp, xw, _mm_store_si128);
			END
		}
	} else {
		for(;;){
			BEGIN
			BREAK_OPEN
			wp = (wchar_t *)_MCFCRT_rep_movsw(_MCFCRT_NULLPTR, (uint16_t *)wp, (const uint16_t *)arp - 32, (unsigned)__builtin_ctzl(mask));
			BREAK_CLOSE
			wp = __MCFCRT_xmmstore_4(wp, xw, _mm_storeu_si128);
			END
		}
	}
end:
	*wp = 0;
	return wp;
}
