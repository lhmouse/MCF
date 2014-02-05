// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

extern wchar_t *_wcscpyout(wchar_t *restrict dst, const wchar_t *restrict src);

wchar_t *wcscpy(wchar_t *restrict dst, const wchar_t *restrict src){
	_wcscpyout(dst, src);
	return dst;
}
