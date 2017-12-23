// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "_memcpy_impl.h"
#include "../../env/expect.h"
#include "../../env/inline_mem.h"

#undef memcpy

void *memcpy(void *restrict s1, const void *restrict s2, size_t n){
#ifndef NDEBUG
	_MCFCRT_inline_mempset_fwd(s1, 0xEF, n);
#endif
	if(_MCFCRT_EXPECT(__MCFCRT_memcpy_is_small_enough(n))){
		__MCFCRT_memcpy_small_fwd(s1, s2, n);
	} else {
		__MCFCRT_memcpy_large_fwd(s1, s2, n);
	}
	return s1;
}
