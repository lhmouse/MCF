// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

extern void *memcpy(void *restrict s1, const void *restrict s2, size_t n);

wchar_t *wmemcpy(wchar_t *restrict s1, const wchar_t *restrict s2, size_t n){
	return memcpy(s1, s2, sizeof(wchar_t) * n);
}
