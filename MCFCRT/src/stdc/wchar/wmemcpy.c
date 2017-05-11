// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../string/_memcpy_impl.h"

#undef wmemcpy

wchar_t *wmemcpy(wchar_t *restrict s1, const wchar_t *restrict s2, size_t n){
	__MCFCRT_memcpy_impl_fwd(s1, s2, sizeof(wchar_t) * n);
	return s1;
}
