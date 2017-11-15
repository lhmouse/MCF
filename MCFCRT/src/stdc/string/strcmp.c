// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../env/expect.h"
#include "_sse2.h"
#include "_ssse3.h"

#undef strcmp

int strcmp(const char *s1, const char *s2){
	// 如果 arp1 和 arp2 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	const unsigned char *arp1 = (const unsigned char *)((uintptr_t)s1 & (uintptr_t)-32);
	const unsigned char *arp2 = (const unsigned char *)((uintptr_t)s2 & (uintptr_t)-32);
	const unsigned align = (unsigned)(32 - ((const unsigned char *)s1 - arp1) + ((const unsigned char *)s2 - arp2));
	__m128i xz[1];
	__MCFCRT_xmmsetz(xz);

	__m128i s2v[6];
	bool s2z;
	__m128i xw[2], xc[2];
	uint32_t mask;
	ptrdiff_t dist;
//=============================================================================
#define BEGIN	\
	arp1 = __MCFCRT_xmmload_2(xw, arp1, _mm_load_si128);	\
	for(unsigned i = 0; i < 4; i += 2){	\
		__MCFCRT_xmmload_2(s2v + i, s2v + i + 2, _mm_load_si128);	\
	}	\
	if(_MCFCRT_EXPECT(!s2z)){	\
		arp2 = __MCFCRT_xmmload_2(s2v + 4, arp2, _mm_load_si128);	\
		mask = __MCFCRT_xmmcmp_21b(s2v + 4, xz);	\
		s2z = mask != 0;	\
	}	\
	__MCFCRT_xmmalign_2(xc, s2v, align);	\
	mask = ~__MCFCRT_xmmcmpandn_221b(xw, xc, xz);
#define END	\
	if(_MCFCRT_EXPECT_NOT(mask != 0)){	\
		goto end;	\
	}
//=============================================================================
	__MCFCRT_xmmsetz_2(s2v + 2);
	arp2 = __MCFCRT_xmmload_2(s2v + 4, arp2, _mm_load_si128);
	mask = __MCFCRT_xmmcmp_21b(s2v + 4, xz);
	dist = (const unsigned char *)s2 - (arp2 - 32);
	mask &= (uint32_t)-1 << dist;
	s2z = mask != 0;
	BEGIN
	dist = (const unsigned char *)s1 - (arp1 - 32);
	mask &= (uint32_t)-1 << dist;
	for(;;){
		END
		BEGIN
	}
end:
	arp1 = arp1 - 32 + (unsigned)__builtin_ctzl(mask);
	arp2 = arp1 - (const unsigned char *)s1 + (const unsigned char *)s2;
	if(*arp1 == *arp2){
		goto end_equal;
	}
	return (*arp1 < *arp2) ? -1 : 1;
end_equal:
	return 0;
}
