// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "stpcpy.h"
#include "../env/expect.h"
#include "../env/xassert.h"
#include "../stdc/string/_sse2.h"
#include "rep_movs.h"

char *_MCFCRT_stpcpy(char *restrict s1, const char *restrict s2){
	// 如果 arp 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	char *wp = s1;
	const char *rp = s2;
	const char *arp = (const char *)((uintptr_t)s2 & (uintptr_t)-32);
	__m128i xz[1];
	__MCFCRT_xmmsetz(xz);

	__m128i xw[2];
	uint32_t mask;
	ptrdiff_t dist;
//=============================================================================
#define BEGIN	\
	arp = __MCFCRT_xmmload_2(xw, arp, _mm_load_si128);	\
	mask = __MCFCRT_xmmcmp_21b(xw, xz);
#define END	\
	if(_MCFCRT_EXPECT_NOT(mask != 0)){	\
		goto end;	\
	}
//=============================================================================
	BEGIN
	dist = (const char *)s2 - (arp - 32);
	mask &= (uint32_t)-1 << dist;
	END
	wp = (char *)_MCFCRT_rep_movsb(_MCFCRT_NULLPTR, (uint8_t *)wp, (const uint8_t *)rp, (size_t)(arp - rp));
	for(;;){
		rp = arp;
		BEGIN
		END
		wp = __MCFCRT_xmmstore_2(wp, xw, _mm_storeu_si128);
	}
end:
	_MCFCRT_ASSERT(mask != 0);
//	if((mask << dist) != 0){
		arp = arp - 32 + (unsigned)__builtin_ctzl(mask);
		wp = (char *)_MCFCRT_rep_movsb(_MCFCRT_NULLPTR, (uint8_t *)wp, (const uint8_t *)rp, (size_t)(arp - rp));
		goto end_term;
//	}
//	wp = (char *)_MCFCRT_rep_movsb(_MCFCRT_NULLPTR, (uint8_t *)wp, (const uint8_t *)rp, (size_t)(32 - dist));
end_term:
	*wp = 0;
	return wp;
}
