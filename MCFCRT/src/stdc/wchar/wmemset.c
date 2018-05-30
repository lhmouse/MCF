// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

#undef wmemset

extern void * __MCFCRT_memset32(void *s, uint32_t c32, size_t n);

wchar_t * wmemset(wchar_t *s, wchar_t c, size_t n){
	uint32_t c32 = (uint16_t)c;
	c32 += c32 << 16;
	return __MCFCRT_memset32(s, c32, n * sizeof(wchar_t));
}
