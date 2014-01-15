// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_strmanip.h"

__MCF_CRT_EXTERN size_t strcspn(const char *s1, const char *s2){
	unsigned char table[256];
	__MCF_CRT_StrBuildTable(table, s2);
	return (size_t)(__MCF_CRT_StrFindFirstOf(s1, table) - s1);
}
