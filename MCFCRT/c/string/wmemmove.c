// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

extern void *memmove(void *dst, const void *src, size_t cb);

wchar_t *wmemmove(wchar_t *dst, const wchar_t *src, size_t cnt){
	return (wchar_t *)memmove(dst, src, cnt * sizeof(wchar_t));
}
