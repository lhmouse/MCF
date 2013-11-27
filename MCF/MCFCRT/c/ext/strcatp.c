// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

__MCF_CRT_EXTERN void _strcatp(char *__restrict__ *pdst, const char *end, const char *__restrict__ src){
	char *wp = *pdst;
	if(wp != end){
		while(++wp != end){
			const char ch = *(src++);
			if(ch == 0){
				break;
			}
			wp[-1] = ch;
		}
		*(--wp) = 0;
		*pdst = wp;
	}
}
