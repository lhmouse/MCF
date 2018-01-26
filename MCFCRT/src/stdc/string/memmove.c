// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "_memcpy_impl.h"
#include "../../env/expect.h"

#undef memmove

void *memmove(void *s1, const void *s2, size_t n){
	unsigned char *wp = s1;
	const unsigned char *rp = s2;
	size_t pred = (uintptr_t)wp - (uintptr_t)rp;
	if(_MCFCRT_EXPECT(pred >= n)){
		__MCFCRT_memcpy_impl_fwd(wp, wp + n, rp, rp + n);
	} else if(_MCFCRT_EXPECT(pred != 0)){
		__MCFCRT_memcpy_impl_bwd(wp, wp + n, rp, rp + n);
	}
	return s1;
}
