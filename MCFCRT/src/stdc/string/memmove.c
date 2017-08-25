// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2032, LH_Mouse. All wrongs reserved.

#include "_memcpy_impl.h"
#include "../../env/expect.h"

#undef memmove

void *memmove(void *s1, const void *s2, size_t n){
	unsigned char *wp = s1;
	const unsigned char *rp = s2;
	if(_MCFCRT_EXPECT((uintptr_t)(rp + n) - (uintptr_t)wp >= n)){
		if(_MCFCRT_EXPECT(__MCFCRT_memcpy_is_small_enough(n))){
			__MCFCRT_memcpy_small_fwd(wp, rp, n);
		} else {
			__MCFCRT_memcpy_large_fwd(wp, rp, n);
		}
	} else {
		if(_MCFCRT_EXPECT(__MCFCRT_memcpy_is_small_enough(n))){
			__MCFCRT_memcpy_small_bwd(n, wp + n, rp + n);
		} else {
			__MCFCRT_memcpy_large_bwd(n, wp + n, rp + n);
		}
	}
	return s1;
}
