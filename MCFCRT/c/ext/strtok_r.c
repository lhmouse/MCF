// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../string/_strmanip.h"

char *_strtok_r(char *restrict s1, const char *restrict s2, char **ctx){
	char *from;
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

	unsigned char table[0x100];
	__MCF_CRT_StrBuildTable(table, s2);

	from = __MCF_CRT_StrFindFirstNotOf(from, table);
	if(*from == 0){
		return NULL;
	}

	char *const delim = __MCF_CRT_StrFindFirstOf(from, table);
	if(*delim == 0){
		*ctx = NULL;
	} else {
		*delim = 0;
		*ctx = delim + 1;
	}

	return from;
}
