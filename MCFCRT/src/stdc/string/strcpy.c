// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../ext/stpcpy.h"

#undef strcpy

char *strcpy(char *restrict s1, const char *restrict s2){
	_MCFCRT_stpcpy(s1, s2);
	return s1;
}
