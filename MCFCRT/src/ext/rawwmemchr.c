// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "rawwmemchr.h"
#include "../env/expect.h"
#include "../stdc/string/_sse2.h"

wchar_t * _MCFCRT_rawwmemchr(const wchar_t *s, wchar_t c){
	// 如果 arp 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	const wchar_t *arp = (const wchar_t *)((uintptr_t)s & (uintptr_t)-64);
	__m128i xc[1];
	__MCFCRT_xmmsetw(xc, (uint16_t)c);

	__m128i xw[4];
	uint32_t mask;
	ptrdiff_t dist;
//=============================================================================
#define BEGIN	\
	arp = __MCFCRT_xmmload_4(xw, arp, _mm_load_si128);	\
	mask = __MCFCRT_xmmcmp_41w(xw, xc);
#define END	\
	if(_MCFCRT_EXPECT_NOT(mask != 0)){	\
		goto end;	\
	}
//=============================================================================
	BEGIN
	dist = (const wchar_t *)s - (arp - 32);
	mask &= (uint32_t)-1 << dist;
	for(;;){
		END
		BEGIN
	}
end:
	arp = arp - 32 + (unsigned)__builtin_ctzl(mask);
	return (wchar_t *)arp;
}
