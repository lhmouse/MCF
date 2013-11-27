// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_STRMANIP_H__
#define __MCF_CRT_STRMANIP_H__

#include "../../env/_crtdef.h"

static inline __attribute__((always_inline)) void __MCF_CRTStrBuildTable(unsigned char *table, const char *s){
	__builtin_memset(table, 0, 0x100);

	const char *rp = s;
	for(;;){
		const char ch = *rp;
		if(ch == 0){
			break;
		}
		table[(unsigned char)ch] = 1;
		++rp;
	}
}

static inline __attribute__((always_inline)) char *__MCF_CRTStrFindFirstOf(const char *s, const unsigned char *table){
	const char *rp = s;
	for(;;){
		const char ch = *rp;
		if(ch == 0){
			break;
		}
		if(table[(unsigned char)ch] != 0){
			break;
		}
		++rp;
	}
	return (char *)rp;
}

static inline __attribute__((always_inline)) char *__MCF_CRTStrFindFirstNotOf(const char *s, const unsigned char *table){
	const char *rp = s;
	for(;;){
		const char ch = *rp;
		if(ch == 0){
			break;
		}
		if(table[(unsigned char)ch] == 0){
			break;
		}
		++rp;
	}
	return (char *)rp;
}

#endif
