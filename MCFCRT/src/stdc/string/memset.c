// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "_memset_impl.h"

#undef memset

void *__MCFCRT_memset32(void *s, uint32_t c32, size_t n){
	unsigned char *wp = s;
	__MCFCRT_memset_impl_fwd(wp, wp + n, c32);
	return s;
}

void *memset(void *s, int c, size_t n){
	uint32_t c32 = (uint8_t)c;
	c32 += c32 <<  8;
	c32 += c32 << 16;
	return __MCFCRT_memset32(s, c32, n);
}
