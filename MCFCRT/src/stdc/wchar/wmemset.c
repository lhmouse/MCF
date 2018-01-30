// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

#undef wmemset

extern void __MCFCRT_memset_impl_fwd(unsigned char *wp, unsigned char *ewp, uint32_t c32);

wchar_t *wmemset(wchar_t *s, wchar_t c, size_t n){
	unsigned char *wp = (void *)s;
	uint32_t c32 = (uint16_t)c;
	c32 += c32 << 16;
	__MCFCRT_memset_impl_fwd(wp, wp + n * sizeof(wchar_t), c32);
	return s;
}
