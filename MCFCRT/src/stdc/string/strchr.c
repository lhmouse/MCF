// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../env/expect.h"
#include "../../ext/rawmemchr.h"
#include "_sse3.h"

#undef strchr

char *strchr(const char *s, int c){
	if(_MCFCRT_EXPECT_NOT((uint8_t)c == 0)){
		return _MCFCRT_rawmemchr(s, 0);
	}
	// 如果 rp 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	register const char *rp = (const char *)((uintptr_t)s & (uintptr_t)-32);
	__m128i xz[1];
	__MCFCRT_xmmsetz(xz);
	__m128i xc[1];
	__MCFCRT_xmmsetb(xc, (uint8_t)c);
	unsigned shift = (unsigned)((const char *)s - rp);
	uint32_t skip = (uint32_t)-1 << shift;
	for(;;){
		__m128i xw[2];
		uint32_t mask;
		__MCFCRT_xmmload_2(xw, rp, _mm_load_si128);
		mask = __MCFCRT_xmmcmp_21b(xw, xz, _mm_cmpeq_epi8) & skip;
		uint32_t zskip = ~mask & (mask - 1);
		mask = __MCFCRT_xmmcmp_21b(xw, xc, _mm_cmpeq_epi8) & skip;
		mask |= ~zskip;
		if(_MCFCRT_EXPECT_NOT(mask != 0)){
			if((mask & zskip) == 0){
				return _MCFCRT_NULLPTR;
			}
			shift = (unsigned)__builtin_ctzl(mask);
			return (char *)rp + shift;
		}
		rp += 32;
		skip = (uint32_t)-1;
	}
}
