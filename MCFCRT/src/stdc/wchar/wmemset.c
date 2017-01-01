// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include <intrin.h>

#undef wmemset

wchar_t *wmemset(wchar_t *s, wchar_t c, size_t n){
	register wchar_t *wp = s;
	wchar_t *const wend = wp + n;
#ifdef _WIN64
	size_t nq = (size_t)(wend - wp) / 4;
	uint64_t q = (uint16_t)c;
	q += (q << 16);
	q += (q << 32);
	__stosq((void *)wp, q, nq);
	wp += nq * 4;
#else
	size_t nd = (size_t)(wend - wp) / 2;
	uint32_t d = (uint16_t)c;
	d += (d << 16);
	__stosd((void *)wp, d, nd);
	wp += nd * 2;
#endif
	size_t nw = (size_t)(wend - wp);
	__stosw((void *)wp, (uint16_t)c, nw);
	return s;
}
