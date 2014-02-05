// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

extern void *memcpy(void *restrict dst, const void *restrict src, size_t cb);

wchar_t *wmemcpy(wchar_t *restrict dst, const wchar_t *restrict src, size_t cnt){
	return (wchar_t *)memcpy(dst, src, cnt * sizeof(wchar_t));
}
