// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../env/expect.h"
#include "../string/_sse2.h"
#include "../string/_ssse3.h"

#undef wcsncmp

int wcsncmp(const wchar_t *s1, const wchar_t *s2, size_t n){
	// 如果 arp1 和 arp2 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	const wchar_t *arp1 = (const wchar_t *)((uintptr_t)s1 & (uintptr_t)-64);
	const wchar_t *arp2 = (const wchar_t *)((uintptr_t)s2 & (uintptr_t)-64);
	const unsigned align = (unsigned)(32 - ((const wchar_t *)s1 - arp1) + ((const wchar_t *)s2 - arp2));
	__m128i xz[1];
	__MCFCRT_xmmsetz(xz);

	__m128i s2v[12];
	bool s2z;
	__m128i xw[4], xc[4];
	uint32_t mask;
	ptrdiff_t dist;
//=============================================================================
#define BEGIN	\
	arp1 = __MCFCRT_xmmload_4(xw, arp1, _mm_load_si128);	\
	for(unsigned i = 0; i < 8; i += 4){	\
		__MCFCRT_xmmload_4(s2v + i, s2v + i + 4, _mm_load_si128);	\
	}	\
	if(_MCFCRT_EXPECT(!s2z)){	\
		arp2 = __MCFCRT_xmmload_4(s2v + 8, arp2, _mm_load_si128);	\
		mask = __MCFCRT_xmmcmp_41w(s2v + 8, xz);	\
		s2z = (mask != 0) || (arp2 >= (const wchar_t *)s2 + n);	\
	}	\
	__MCFCRT_xmmalign_4(xc, s2v, align);	\
	mask = ~__MCFCRT_xmmcmpandn_441w(xw, xc, xz);
#define END	\
	dist = arp1 - ((const wchar_t *)s1 + n);	\
	if(_MCFCRT_EXPECT_NOT(dist >= 0)){	\
		goto end_trunc;	\
	}	\
	dist = 0;	\
	if(_MCFCRT_EXPECT_NOT(mask != 0)){	\
		goto end;	\
	}
//=============================================================================
	if(_MCFCRT_EXPECT_NOT(n == 0)){
		goto end_equal;
	}
	__MCFCRT_xmmsetz_4(s2v + 4);
	arp2 = __MCFCRT_xmmload_4(s2v + 8, arp2, _mm_load_si128);
	mask = __MCFCRT_xmmcmp_41w(s2v + 8, xz);
	dist = (const wchar_t *)s2 - (arp2 - 32);
	mask &= (uint32_t)-1 << dist;
	s2z = (mask != 0) || (arp2 >= (const wchar_t *)s2 + n);
	BEGIN
	dist = (const wchar_t *)s1 - (arp1 - 32);
	mask &= (uint32_t)-1 << dist;
	for(;;){
		END
		BEGIN
	}
end_trunc:
	mask |= ~((uint32_t)-1 >> dist);
end:
	if((mask << dist) != 0){
		arp1 = arp1 - 32 + (unsigned)__builtin_ctzl(mask);
		arp2 = arp1 - (const wchar_t *)s1 + (const wchar_t *)s2;
		if(*arp1 == *arp2){
			goto end_equal;
		}
		return (*arp1 < *arp2) ? -1 : 1;
	}
end_equal:
	return 0;
}
