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
	register wchar_t *wp __asm__("di") = s1;
	register const wchar_t *rp __asm__("si") = s2;
	const wchar_t *arp = (const wchar_t *)((uintptr_t)s2 & (uintptr_t)-64);
	__m128i xz[1];
	__MCFCRT_xmmsetz(xz);

	__m128i xw[4];
	uint32_t mask;
	ptrdiff_t wdist;
//=============================================================================
#define BEGIN	\
	arp = __MCFCRT_xmmload_4(xw, arp, _mm_load_si128);	\
	mask = __MCFCRT_xmmcmp_41w(xw, xz);
#define END	\
	wdist = arp - (s2 + (es1 - 1 - s1));	\
	if(_MCFCRT_EXPECT_NOT(wdist > 0)){	\
		goto end_trunc;	\
	}	\
	if(_MCFCRT_EXPECT_NOT(mask != 0)){	\
		goto end;	\
	}
//=============================================================================
	BEGIN
	wdist = (const wchar_t *)s2 - (arp - 32);
	mask &= (uint32_t)-1 << wdist;
	END
	wp = (wchar_t *)_MCFCRT_rep_movsw(_MCFCRT_NULLPTR, (uint16_t *)wp, (const uint16_t *)rp, (size_t)(arp - rp));
	if(((uintptr_t)wp & ~(uintptr_t)-16) == 0){
		for(;;){
			rp = arp;
			BEGIN
			END
			wp = __MCFCRT_xmmstore_4(wp, xw, _mm_store_si128);
		}
	} else {
		for(;;){
			rp = arp;
			BEGIN
			END
			wp = __MCFCRT_xmmstore_4(wp, xw, _mm_storeu_si128);
		}
	}
end_trunc:
	mask |= ~((uint32_t)-1 >> wdist);
end:
	_MCFCRT_ASSERT(mask != 0);
	arp = arp - 32 + (unsigned)__builtin_ctzl(mask);
	wp = (wchar_t *)_MCFCRT_rep_movsw(_MCFCRT_NULLPTR, (uint16_t *)wp, (const uint16_t *)rp, (size_t)(arp - rp));
	*wp = 0;
	return wp;
}
