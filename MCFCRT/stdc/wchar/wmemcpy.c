// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

wchar_t *wmemcpy(wchar_t *restrict dst, const wchar_t *restrict src, size_t cnt){
	return memcpy(dst, src, cnt * sizeof(wchar_t));
}
