// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

__MCF_CRT_EXTERN void *memcpy(void *__restrict__ dst, const void *__restrict__ src, size_t cb);

__MCF_CRT_EXTERN wchar_t *wmemcpy(wchar_t *__restrict__ dst, const wchar_t *__restrict__ src, size_t cnt){
	return (wchar_t *)memcpy(dst, src, cnt * sizeof(wchar_t));
}
