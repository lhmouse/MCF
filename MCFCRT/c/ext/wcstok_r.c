// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include <wchar.h>

wchar_t *_wcstok_r(wchar_t *restrict s1, const wchar_t *restrict s2, wchar_t **ctx){
	wchar_t *from;
	if(s1){
		from = s1;
	} else if(*ctx){
		from = *ctx;
	} else {
		return NULL;
	}
	if(*from == 0){
		return NULL;
	}

	from += wcsspn(from, s2);
	if(*from == 0){
		return NULL;
	}

	wchar_t *const delim = wcspbrk(from, s2);
	if(!delim || !*delim){
		*ctx = NULL;
	} else {
		*delim = 0;
		*ctx = delim + 1;
	}

	return from;
}
