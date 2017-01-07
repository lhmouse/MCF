// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../ext/rep_stos.h"

#undef memset

void *memset(void *s, int c, size_t n){
	register char *wp = s;
	char *const wend = wp + n;
	uint8_t b = (uint8_t)c;
#ifdef _WIN64
	size_t nq = (size_t)(wend - wp) / 8;
	uint64_t q = b;
	q += (q <<  8);
	q += (q << 16);
	q += (q << 32);
	wp = _MCFCRT_rep_stosq(wp, q, nq);
#else
	size_t nd = (size_t)(wend - wp) / 4;
	uint32_t d = b;
	d += (d <<  8);
	d += (d << 16);
	wp = _MCFCRT_rep_stosd(wp, d, nd);
#endif
	size_t nb = (size_t)(wend - wp);
	wp = _MCFCRT_rep_stosb(wp, b, nb);
	return s;
}
