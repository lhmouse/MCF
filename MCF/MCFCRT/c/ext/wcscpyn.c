// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

__MCF_CRT_EXTERN wchar_t *_wcscpyn(wchar_t *__restrict__ dst, size_t cap, const wchar_t *__restrict__ src){
	if(cap != 0){
		while(--cap != 0){
			const wchar_t ch = *(src++);
			if(ch == 0){
				break;
			}
			*(dst++) = ch;
		}
		*dst = 0;
	}
	return dst;
}
