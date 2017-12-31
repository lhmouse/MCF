// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../ext/wcpcpy.h"

#undef wcscpy

wchar_t *wcscpy(wchar_t *restrict s1, const wchar_t *restrict s2){
	_MCFCRT_wcpcpy(s1, s2);
	return s1;
}
