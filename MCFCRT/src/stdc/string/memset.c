// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../ext/rep_stos.h"

#undef memset

void *memset(void *s, int c, size_t n){
	register unsigned char *wp = s;
	uintptr_t word;
	word = (uint8_t)c;
#ifdef _WIN64
	word += word <<  8;
	word += word << 16;
	word += word << 32;
	wp = (unsigned char *)_MCFCRT_rep_stosq((uint64_t *)wp, (uint64_t)word, n / 8);
	wp = (unsigned char *)_MCFCRT_rep_stosb( (uint8_t *)wp,  (uint8_t)word, n % 8);
#else
	word += word <<  8;
	word += word << 16;
	wp = (unsigned char *)_MCFCRT_rep_stosd((uint32_t *)wp, (uint32_t)word, n / 4);
	wp = (unsigned char *)_MCFCRT_rep_stosb( (uint8_t *)wp,  (uint8_t)word, n % 4);
#endif
	return s;
}
