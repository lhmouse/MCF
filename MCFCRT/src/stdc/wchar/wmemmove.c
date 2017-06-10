// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../string/_memcpy_impl.h"
#include "../../env/expect.h"

#undef wmemmove

wchar_t *wmemmove(wchar_t *restrict s1, const wchar_t *restrict s2, size_t n){
	if(_MCFCRT_EXPECT((uintptr_t)s2 + n - (uintptr_t)s1 >= n)){
		__MCFCRT_memcpy_impl_fwd(s1, s2, sizeof(wchar_t) * n);
	} else {
		__MCFCRT_memcpy_impl_bkwd(s1, s2, sizeof(wchar_t) * n);
	}
	return s1;
}
