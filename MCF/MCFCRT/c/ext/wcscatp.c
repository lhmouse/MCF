// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

__MCF_CRT_EXTERN void _wcscatp(wchar_t *__restrict__ *pdst, const wchar_t *end, const wchar_t *__restrict__ src){
	wchar_t *wp = *pdst;
	if(wp != end){
		while(++wp != end){
			const wchar_t ch = *(src++);
			if(ch == 0){
				break;
			}
			wp[-1] = ch;
		}
		*(--wp) = 0;
		*pdst = wp;
	}
}
