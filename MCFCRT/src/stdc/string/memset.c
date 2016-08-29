// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include <intrin.h>

void *memset(void *s, int c, size_t n){
	register char *wp = s;
	char *const wend = wp + n;
#ifdef _WIN64
	size_t nq = (size_t)(wend - wp) / 8;
	uint64_t q = (uint8_t)c;
	q += (q <<  8);
	q += (q << 16);
	q += (q << 32);
	__stosq((void *)wp, q, nq);
	wp += nq * 8;
#else
	size_t nd = (size_t)(wend - wp) / 4;
	uint32_t d = (uint8_t)c;
	d += (d <<  8);
	d += (d << 16);
	__stosd((void *)wp, d, nd);
	wp += nd * 4;
#endif
	size_t nb = (size_t)(wend - wp);
	__stosb((void *)wp, (uint8_t)c, nb);
	return s;
}
