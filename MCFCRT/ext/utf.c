// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "utf.h"

char32_t _MCFCRT_DecodeUtf8 (const char **ppchRead){
	register const char *pchRead = *ppchRead;
	register uint_fast32_t u32CodePoint = (uint_fast32_t)*(pchRead++);
	if(u32CodePoint < 0x80){ // 7 位
		//
	} else if((u32CodePoint & 0xE0) == 0xC0){ // 11 位
		u32CodePoint &= 0x1F;
		u32CodePoint <<= 6;
		u32CodePoint |= ((uint_fast32_t)*(pchRead++) & 0x3F)      ;
	} else if((u32CodePoint & 0xF0) == 0xE0){ // 16 位
		u32CodePoint &= 0x0F;
		u32CodePoint <<= 12;
		u32CodePoint |= ((uint_fast32_t)*(pchRead++) & 0x3F) <<  6;
		u32CodePoint |= ((uint_fast32_t)*(pchRead++) & 0x3F)      ;
	} else { // 21 位
		u32CodePoint &= 0x0F;
		u32CodePoint <<= 18;
		u32CodePoint |= ((uint_fast32_t)*(pchRead++) & 0x3F) << 12;
		u32CodePoint |= ((uint_fast32_t)*(pchRead++) & 0x3F) <<  6;
		u32CodePoint |= ((uint_fast32_t)*(pchRead++) & 0x3F)      ;
	}
	*ppchRead = pchRead;
	return (char32_t)u32CodePoint;
}
char32_t _MCFCRT_DecodeUtf16(const char16_t **ppc16Read){
	register const char16_t *pc16Read = *ppc16Read;
	register uint_fast32_t u32CodePoint = (uint_fast32_t)*(pc16Read++);
	if((u32CodePoint < 0xD800) || (0xDC00 <= u32CodePoint)){ // 1 编码单元
		//
	} else { // 2 编码单元
		u32CodePoint &= 0x3FF;
		u32CodePoint <<= 10;
		u32CodePoint |= ((uint_fast32_t)*(pc16Read++) & 0x3FF);
		u32CodePoint += 0x10000;
	}
	*ppc16Read = pc16Read;
	return (char32_t)u32CodePoint;
}

void _MCFCRT_EncodeUtf8 (char **ppchWrite, char32_t c32CodePoint){
	register char *pchWrite = *ppchWrite;
	register uint_fast32_t u32CodePoint = (uint_fast32_t)c32CodePoint;
	if(u32CodePoint < 0x80){ // 7 位
		*(pchWrite++) = (char)(((u32CodePoint      )       )       );
	} else if(u32CodePoint < 0x800){ // 11 位 = 5 + 6
		*(pchWrite++) = (char)(((u32CodePoint >>  6) & 0x1F) | 0xC0);
		*(pchWrite++) = (char)(((u32CodePoint      ) & 0x3F) | 0x80);
	} else if(u32CodePoint < 0x10000){ // 16 位 = 4 + 6 + 6
		*(pchWrite++) = (char)(((u32CodePoint >> 12) & 0x0F) | 0xE0);
		*(pchWrite++) = (char)(((u32CodePoint >>  6) & 0x3F) | 0x80);
		*(pchWrite++) = (char)(((u32CodePoint      ) & 0x3F) | 0x80);
	} else { // 21 位 = 3 + 6 + 6 + 6
		*(pchWrite++) = (char)(((u32CodePoint >> 18) & 0x07) | 0xF0);
		*(pchWrite++) = (char)(((u32CodePoint >> 12) & 0x3F) | 0x80);
		*(pchWrite++) = (char)(((u32CodePoint >>  6) & 0x3F) | 0x80);
		*(pchWrite++) = (char)(((u32CodePoint      ) & 0x3F) | 0x80);
	}
	*ppchWrite = pchWrite;
}
void _MCFCRT_EncodeUtf16(char16_t **ppc16Write, char32_t c32CodePoint){
	register char16_t *pc16Write = *ppc16Write;
	register uint_fast32_t u32CodePoint = (uint_fast32_t)c32CodePoint;
	if(u32CodePoint < 0x10000){ // 1 编码单元
		*(pc16Write++) = (char16_t)(((u32CodePoint        )         )         );
	} else { // 2 编码单元
		u32CodePoint -= 0x10000;
		*(pc16Write++) = (char16_t)((((u32CodePoint) >> 10) & 0x03FF) | 0xD800);
		*(pc16Write++) = (char16_t)((((u32CodePoint)      ) & 0x03FF) | 0xDC00);
	}
	*ppc16Write = pc16Write;
}
