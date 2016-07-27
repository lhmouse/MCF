// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

wchar_t *wmemmove(wchar_t *dst, const wchar_t *src, size_t cnt){
	return memmove(dst, src, cnt * sizeof(wchar_t));
}
