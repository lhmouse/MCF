// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../string/_memcpyset_inl.h"

wchar_t *wmemmove(wchar_t *restrict s1, const wchar_t *restrict s2, size_t n){
	if(s1 < s2){
		__MCFCRT_CopyForward(s1, s2, sizeof(wchar_t) * n);
	} else {
		__MCFCRT_CopyBackward(s1, s2, sizeof(wchar_t) * n);
	}
	return s1;
}
