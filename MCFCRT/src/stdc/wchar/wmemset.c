// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../ext/rep_stos.h"

#undef wmemset

wchar_t *wmemset(wchar_t *s, wchar_t c, size_t n){
	register wchar_t *wp __asm__("di") = s;
	uintptr_t word;
	word = (uint16_t)c;
#ifdef _WIN64
	word += word << 16;
	word += word << 32;
	wp = (wchar_t *)_MCFCRT_rep_stosq((uint64_t *)wp, (uint64_t)word, n / 4);
	wp = (wchar_t *)_MCFCRT_rep_stosw((uint16_t *)wp, (uint16_t)word, n % 4);
#else
	word += word << 16;
	wp = (wchar_t *)_MCFCRT_rep_stosd((uint32_t *)wp, (uint32_t)word, n / 2);
	wp = (wchar_t *)_MCFCRT_rep_stosw((uint16_t *)wp, (uint16_t)word, n % 2);
#endif
	return s;
}
