// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "_memcpy_impl.h"
#include "../../env/expect.h"
#include "../../env/inline_mem.h"

#undef memcpy

void *memcpy(void *restrict s1, const void *restrict s2, size_t n){
	unsigned char *wp = s1;
	const unsigned char *rp = s2;
#ifndef NDEBUG
	_MCFCRT_inline_mempset_fwd(s1, 0xEF, n);
	__MCFCRT_memcpy_impl_bwd(wp, wp + n, rp, rp + n);
#else
	__MCFCRT_memcpy_impl_fwd(wp, wp + n, rp, rp + n);
#endif
	return s1;
}
