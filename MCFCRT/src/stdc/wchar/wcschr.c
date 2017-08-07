// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../env/expect.h"
#include "../string/_sse3.h"

#undef wcschr

wchar_t *wcschr(const wchar_t *s, wchar_t c){
	// 如果 arp 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	register const wchar_t *arp = (const wchar_t *)((uintptr_t)s & (uintptr_t)-64);
	__m128i xc[1];
	__MCFCRT_xmmsetw(xc, (uint16_t)c);
	__m128i xz[1];
	__MCFCRT_xmmsetz(xz);
//=============================================================================
#define LOOP_BODY(skip_)	\
	{	\
		__m128i xw[4];	\
		uint32_t mask;	\
		arp = __MCFCRT_xmmload_4(xw, arp, _mm_load_si128);	\
		mask = __MCFCRT_xmmcmpor_411w(xw, xc, xz, _mm_cmpeq_epi16);	\
		mask &= (skip_);	\
		__builtin_prefetch(arp + 64, 0, 0);	\
		if(_MCFCRT_EXPECT_NOT(mask != 0)){	\
			const wchar_t *const rp = arp - 32 + (unsigned)__builtin_ctzl(mask);	\
			if(*rp != (wchar_t)c){	\
				return _MCFCRT_NULLPTR;	\
			}	\
			return (wchar_t *)rp;	\
		}	\
	}
//=============================================================================
	LOOP_BODY((uint32_t)-1 << ((const wchar_t *)s - arp))
	for(;;){
		LOOP_BODY((uint32_t)-1)
	}
}
