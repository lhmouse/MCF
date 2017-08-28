// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../env/expect.h"

#undef wmemcmp

static inline uintptr_t wswap_ptr(uintptr_t w){
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	return _Generic(w,
		uint32_t: __builtin_bswap32((uint32_t)(((w << 8) & 0xFF00FF00        ) | ((w & 0xFF00FF00        ) >> 8))),
		uint64_t: __builtin_bswap64((uint64_t)(((w << 8) & 0xFF00FF00FF00FF00) | ((w & 0xFF00FF00FF00FF00) >> 8))));
#else
	return w;
#endif
}

int wmemcmp(const wchar_t *s1, const wchar_t *s2, size_t n){
	const wchar_t *rp1 = s1;
	const wchar_t *rp2 = s2;
	size_t rem = n / (sizeof(uintptr_t) / sizeof(wchar_t));
	if(_MCFCRT_EXPECT_NOT(rem != 0)){
		switch((rem - 1) % 32){
			uintptr_t w, c;
		diff_wc:
			w = wswap_ptr(w);
			c = wswap_ptr(c);
			return (w < c) ? -1 : 1;
#define STEP(k_)	\
				__attribute__((__fallthrough__));	\
		case (k_):	\
				__builtin_memcpy(&w, rp1, sizeof(w));	\
				__builtin_memcpy(&c, rp2, sizeof(c));	\
				--rem;	\
				if(_MCFCRT_EXPECT_NOT(w != c)){	\
					goto diff_wc;	\
				}	\
				rp1 += sizeof(w) / sizeof(wchar_t);	\
				rp2 += sizeof(c) / sizeof(wchar_t);
//=============================================================================
			do {
		STEP(037)  STEP(036)  STEP(035)  STEP(034)  STEP(033)  STEP(032)  STEP(031)  STEP(030)
		STEP(027)  STEP(026)  STEP(025)  STEP(024)  STEP(023)  STEP(022)  STEP(021)  STEP(020)
		STEP(017)  STEP(016)  STEP(015)  STEP(014)  STEP(013)  STEP(012)  STEP(011)  STEP(010)
		STEP(007)  STEP(006)  STEP(005)  STEP(004)  STEP(003)  STEP(002)  STEP(001)  STEP(000)
			} while(_MCFCRT_EXPECT(rem != 0));
//=============================================================================
#undef STEP
		}
	}
	rem = n % (sizeof(uintptr_t) / sizeof(wchar_t));
	while(_MCFCRT_EXPECT(rem != 0)){
		if(*rp1 != *rp2){
			return (*rp1 < *rp2) ? -1 : 1;
		}
		++rp1;
		++rp2;
		--rem;
	}
	return 0;
}
