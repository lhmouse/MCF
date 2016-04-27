// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../ext/stpcpy.h"

char *strcpy(char *s1, const char *s2){
	_MCFCRT_stpcpy(s1, s2);
	return s1;
}
