// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

__MCF_CRT_EXTERN char *_strcpyn(char *__restrict__ dst, size_t cap, const char *__restrict__ src){
	if(cap != 0){
		while(--cap != 0){
			const char ch = *(src++);
			if(ch == 0){
				break;
			}
			*(dst++) = ch;
		}
		*dst = 0;
	}
	return dst;
}
