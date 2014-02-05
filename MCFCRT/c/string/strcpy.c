// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

extern char *_strcpyout(char *restrict dst, const char *restrict src);

char *strcpy(char *restrict dst, const char *restrict src){
	_strcpyout(dst, src);
	return dst;
}
