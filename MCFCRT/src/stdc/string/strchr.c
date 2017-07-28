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
	__m128i xzc[2];
	__MCFCRT_xmmsetz(xzc + 0);
	__MCFCRT_xmmsetb(xzc + 1, (uint8_t)c);
	unsigned shift = (unsigned)((const char *)s - rp);
	uint32_t skip = (uint32_t)-1 << shift;
	for(;;){
		__m128i xw[2];
		uint32_t mask;
		rp = __MCFCRT_xmmload_2(xw, rp, _mm_load_si128);
		mask = __MCFCRT_xmmcmpor_22b(xw, xzc, _mm_cmpeq_epi8) & skip;
		__builtin_prefetch(rp + 64, 0, 0);
		if(_MCFCRT_EXPECT_NOT(mask != 0)){
			shift = (unsigned)__builtin_ctzl(mask);
			if(*(rp - 32 + shift) == 0){
				return _MCFCRT_NULLPTR;
			}
			return (char *)rp - 32 + shift;
		}
		skip = (uint32_t)-1;
	}
}
