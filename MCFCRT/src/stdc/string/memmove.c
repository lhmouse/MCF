// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../env/expect.h"

#undef memmove

extern void __MCFCRT_memcpy_impl_bwd(unsigned char *bwp, unsigned char *ewp, const unsigned char *brp, const unsigned char *erp);
extern void __MCFCRT_memcpy_impl_fwd(unsigned char *bwp, unsigned char *ewp, const unsigned char *brp, const unsigned char *erp);

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
