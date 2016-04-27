// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "utf.h"

char32_t _MCFCRT_DecodeUtf8 (const char **ppchRead){
	uint_fast32_t u32CodePoint = (uint_fast32_t)*((*ppchRead)++);
	if(u32CodePoint >= 0x80){
		if((u32CodePoint & 0xE0) == 0xC0){ // 11 位
			u32CodePoint &= 0x1F;
			u32CodePoint <<= 6;
			u32CodePoint |= ((uint_fast32_t)*((*ppchRead)++) & 0x3F      );
		} else if((u32CodePoint & 0xF0) == 0xE0){ // 16 位
			u32CodePoint &= 0x0F;
			u32CodePoint <<= 12;
			u32CodePoint |= ((uint_fast32_t)*((*ppchRead)++) & 0x3F) <<  6;
			u32CodePoint |= ((uint_fast32_t)*((*ppchRead)++) & 0x3F)      ;
		} else { // 21 位
			u32CodePoint &= 0x0F;
			u32CodePoint <<= 18;
			u32CodePoint |= ((uint_fast32_t)*((*ppchRead)++) & 0x3F) << 12;
			u32CodePoint |= ((uint_fast32_t)*((*ppchRead)++) & 0x3F) <<  6;
			u32CodePoint |= ((uint_fast32_t)*((*ppchRead)++) & 0x3F)      ;
		}
	}
	return (char32_t)u32CodePoint;
}
char32_t _MCFCRT_DecodeUtf16(const char16_t **ppc16Read){
	uint_fast32_t u32CodePoint = (uint_fast32_t)*((*ppc16Read)++);
	if((0xD800 <= u32CodePoint) && (u32CodePoint < 0xDC00)){ // 2 码点
		u32CodePoint &= 0x3FF;
		u32CodePoint <<= 10;
		u32CodePoint |= ((uint_fast32_t)*((*ppc16Read)++) & 0x3FF);
		u32CodePoint += 0x10000;
	}
	return (char32_t)u32CodePoint;
}

void _MCFCRT_EncodeUtf8 (char **ppchWrite, char32_t c32CodePoint){
	uint_fast32_t u32CodePoint = (uint_fast32_t)c32CodePoint;
	if(u32CodePoint < 0x80){ // 7 位
		*((*ppchWrite)++) = (char)(((u32CodePoint      ) & 0xFF)       );
	} else if(u32CodePoint < 0x800){ // 11 位 = 5 + 6
		*((*ppchWrite)++) = (char)(((u32CodePoint >>  6) & 0x1F) | 0xC0);
		*((*ppchWrite)++) = (char)(((u32CodePoint      ) & 0x3F) | 0x80);
	} else if(u32CodePoint < 0x10000){ // 16 位 = 4 + 6 + 6
		*((*ppchWrite)++) = (char)(((u32CodePoint >> 12) & 0x0F) | 0xE0);
		*((*ppchWrite)++) = (char)(((u32CodePoint >>  6) & 0x3F) | 0x80);
		*((*ppchWrite)++) = (char)(((u32CodePoint      ) & 0x3F) | 0x80);
	} else { // 21 位 = 3 + 6 + 6 + 6
		*((*ppchWrite)++) = (char)(((u32CodePoint >> 18) & 0x07) | 0xF0);
		*((*ppchWrite)++) = (char)(((u32CodePoint >> 12) & 0x3F) | 0x80);
		*((*ppchWrite)++) = (char)(((u32CodePoint >>  6) & 0x3F) | 0x80);
		*((*ppchWrite)++) = (char)(((u32CodePoint      ) & 0x3F) | 0x80);
	}
}
void _MCFCRT_EncodeUtf16(char16_t **ppc16Write, char32_t c32CodePoint){
	uint_fast32_t u32CodePoint = (uint_fast32_t)c32CodePoint;
	if(u32CodePoint < 0x10000){ // 1 码点
		*((*ppc16Write)++) = (char16_t)(((u32CodePoint      ) & 0xFFFF)       );
	} else { // 2 码点
		u32CodePoint -= 0x10000;
		*((*ppc16Write)++) = (char16_t)((((u32CodePoint) >> 10) & 0x03FF) | 0xD800);
		*((*ppc16Write)++) = (char16_t)((((u32CodePoint)      ) & 0x03FF) | 0xDC00);
	}
}
