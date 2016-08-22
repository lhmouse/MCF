// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "utf.h"

_MCFCRT_Utf8CodeUnitType _MCFCRT_GetUtf8CodeUnitType(char chCodeUnit){
	register uint_fast32_t u32Temp = (uint8_t)chCodeUnit;
	if(u32Temp < 0x80){
		return _MCFCRT_kUtf8Ascii;
	} else if(u32Temp < 0xC0){
		return _MCFCRT_kUtf8Trailing;
	} else if(u32Temp < 0xE0){
		return _MCFCRT_kUtf8LeadingTwo;
	} else if(u32Temp < 0xF0){
		return _MCFCRT_kUtf8LeadingThree;
	} else if(u32Temp < 0xF8){
		return _MCFCRT_kUtf8LeadingFour;
	} else {
		return _MCFCRT_kUtf8Reserved;
	}
}

_MCFCRT_Utf16CodeUnitType _MCFCRT_GetUtf16CodeUnitType(char16_t c16CodeUnit){
	register uint_fast32_t u32Temp = (uint16_t)c16CodeUnit;
	if(u32Temp < 0xD800){
		return _MCFCRT_kUtf16Normal;
	} else if(u32Temp < 0xDC00){
		return _MCFCRT_kUtf16LeadingSurrogate;
	} else if(u32Temp < 0xE000){
		return _MCFCRT_kUtf16TrailingSurrogate;
	} else {
		return _MCFCRT_kUtf16Normal;
	}
}

char32_t _MCFCRT_DecodeUtf8(const char **ppchRead, const char *pchReadEnd){
	register const char *pchRead = *ppchRead;
	if(pchRead == pchReadEnd){
		return _MCFCRT_UTF_PARTIAL_DATA;
	}
	register uint_fast32_t u32Temp = (uint8_t)*(pchRead++);
	if(u32Temp < 0x80){ // 7 位
		//
	} else if(u32Temp < 0xC0){ // 孤立的尾部字节
		return _MCFCRT_UTF_INVALID_INPUT;
	} else if(u32Temp < 0xE0){ // 11 位
		if(pchReadEnd - pchRead < 1){
			return _MCFCRT_UTF_PARTIAL_DATA;
		}
		u32Temp &= 0x1F;
		u32Temp <<= 6;
		uint_fast32_t u32Next = (uint8_t)*(pchRead++);
		if((u32Next < 0x80) || (0xC0 <= u32Next)){
			return _MCFCRT_UTF_INVALID_INPUT;
		}
		u32Temp |= (u32Next & 0x3F);
	} else if(u32Temp < 0xF0){ // 16 位
		if(pchReadEnd - pchRead < 2){
			return _MCFCRT_UTF_PARTIAL_DATA;
		}
		u32Temp &= 0x0F;
		u32Temp <<= 12;
		uint_fast32_t u32Next = (uint8_t)*(pchRead++);
		if((u32Next < 0x80) || (0xC0 <= u32Next)){
			return _MCFCRT_UTF_INVALID_INPUT;
		}
		u32Temp |= (u32Next & 0x3F) << 6;
		u32Next = (uint8_t)*(pchRead++);
		if((u32Next < 0x80) || (0xC0 <= u32Next)){
			return _MCFCRT_UTF_INVALID_INPUT;
		}
		u32Temp |= (u32Next & 0x3F);
		if(u32Temp - 0xD800u < 0x800u){
			return _MCFCRT_UTF_INVALID_INPUT;
		}
	} else if(u32Temp < 0xF8){ // 21 位
		if(pchReadEnd - pchRead < 3){
			return _MCFCRT_UTF_PARTIAL_DATA;
		}
		u32Temp &= 0x07;
		u32Temp <<= 18;
		uint_fast32_t u32Next = (uint8_t)*(pchRead++);
		if((u32Next < 0x80) || (0xC0 <= u32Next)){
			return _MCFCRT_UTF_INVALID_INPUT;
		}
		u32Temp |= (u32Next & 0x3F) << 12;
		u32Next = (uint8_t)*(pchRead++);
		if((u32Next < 0x80) || (0xC0 <= u32Next)){
			return _MCFCRT_UTF_INVALID_INPUT;
		}
		u32Temp |= (u32Next & 0x3F) << 6;
		u32Next = (uint8_t)*(pchRead++);
		if((u32Next < 0x80) || (0xC0 <= u32Next)){
			return _MCFCRT_UTF_INVALID_INPUT;
		}
		u32Temp |= (u32Next & 0x3F);
		if(u32Temp - 0xD800u < 0x800u){
			return _MCFCRT_UTF_INVALID_INPUT;
		}
		if(u32Temp >= 0x110000){
			return _MCFCRT_UTF_INVALID_INPUT;
		}
	} else {
		return _MCFCRT_UTF_INVALID_INPUT;
	}
	*ppchRead = pchRead;
	return (char32_t)u32Temp;
}
char32_t _MCFCRT_DecodeUtf16(const char16_t **ppc16Read, const char16_t *pc16ReadEnd){
	register const char16_t *pc16Read = *ppc16Read;
	if(pc16Read == pc16ReadEnd){
		return _MCFCRT_UTF_PARTIAL_DATA;
	}
	register uint_fast32_t u32Temp = (uint16_t)*(pc16Read++);
	if(u32Temp < 0xD800){ // 1 编码单元
		//
	} else if(u32Temp < 0xDC00){ // 2 编码单元
		if(pc16ReadEnd - pc16Read < 1){
			return _MCFCRT_UTF_PARTIAL_DATA;
		}
		u32Temp &= 0x3FF;
		u32Temp <<= 10;
		uint_fast32_t u32Next = (uint16_t)*(pc16Read++);
		if((u32Next < 0xDC00) || (0xE000 <= u32Next)){
			return _MCFCRT_UTF_INVALID_INPUT;
		}
		u32Temp |= (u32Next & 0x3FF);
		u32Temp += 0x10000;
	} else if(u32Temp < 0xE000){ // 孤立的低位代理
		return _MCFCRT_UTF_INVALID_INPUT;
	} else { // 1 编码单元
		//
	}
	*ppc16Read = pc16Read;
	return (char32_t)u32Temp;
}

char32_t _MCFCRT_EncodeUtf8(char **ppchWrite, char *pchWriteEnd, char32_t c32CodePoint){
	register char *pchWrite = *ppchWrite;
	register uint_fast32_t u32Temp = (uint32_t)c32CodePoint;
	if(u32Temp < 0x80){ // 7 位
		if(pchWriteEnd - pchWrite < 1){
			return _MCFCRT_UTF_BUFFER_TOO_SMALL;
		}
		*(pchWrite++) = (char)u32Temp;
	} else if(u32Temp < 0x800){ // 11 位 = 5 + 6
		if(pchWriteEnd - pchWrite < 2){
			return _MCFCRT_UTF_BUFFER_TOO_SMALL;
		}
		*(pchWrite++) = (char)(((u32Temp >>  6) & 0x1F) | 0xC0);
		*(pchWrite++) = (char)(((u32Temp      ) & 0x3F) | 0x80);
	} else if(u32Temp - 0xD800u < 0x800u){
		return _MCFCRT_UTF_INVALID_INPUT;
	} else if(u32Temp < 0x10000){ // 16 位 = 4 + 6 + 6
		if(pchWriteEnd - pchWrite < 3){
			return _MCFCRT_UTF_BUFFER_TOO_SMALL;
		}
		*(pchWrite++) = (char)(((u32Temp >> 12) & 0x0F) | 0xE0);
		*(pchWrite++) = (char)(((u32Temp >>  6) & 0x3F) | 0x80);
		*(pchWrite++) = (char)(((u32Temp      ) & 0x3F) | 0x80);
	} else if(u32Temp < 0x110000){ // 21 位 = 3 + 6 + 6 + 6
		if(pchWriteEnd - pchWrite < 4){
			return _MCFCRT_UTF_BUFFER_TOO_SMALL;
		}
		*(pchWrite++) = (char)(((u32Temp >> 18) & 0x07) | 0xF0);
		*(pchWrite++) = (char)(((u32Temp >> 12) & 0x3F) | 0x80);
		*(pchWrite++) = (char)(((u32Temp >>  6) & 0x3F) | 0x80);
		*(pchWrite++) = (char)(((u32Temp      ) & 0x3F) | 0x80);
	} else {
		return _MCFCRT_UTF_INVALID_INPUT;
	}
	*ppchWrite = pchWrite;
	return c32CodePoint;
}
char32_t _MCFCRT_EncodeUtf16(char16_t **ppc16Write, char16_t *pc16WriteEnd, char32_t c32CodePoint){
	register char16_t *pc16Write = *ppc16Write;
	register uint_fast32_t u32Temp = (uint32_t)c32CodePoint;
	if(u32Temp - 0xD800u < 0x800u){
		return _MCFCRT_UTF_INVALID_INPUT;
	} else if(u32Temp < 0x10000){ // 1 编码单元
		if(pc16WriteEnd - pc16Write < 1){
			return _MCFCRT_UTF_BUFFER_TOO_SMALL;
		}
		*(pc16Write++) = (char16_t)u32Temp;
	} else if(u32Temp < 0x110000){ // 2 编码单元
		if(pc16WriteEnd - pc16Write < 2){
			return _MCFCRT_UTF_BUFFER_TOO_SMALL;
		}
		u32Temp -= 0x10000;
		*(pc16Write++) = (char16_t)((((u32Temp) >> 10) & 0x03FF) | 0xD800);
		*(pc16Write++) = (char16_t)((((u32Temp)      ) & 0x03FF) | 0xDC00);
	} else {
		return _MCFCRT_UTF_INVALID_INPUT;
	}
	*ppc16Write = pc16Write;
	return c32CodePoint;
}

char32_t _MCFCRT_EncodeUtf8FromUtf16(char **ppchWrite, char *pchWriteEnd, const char16_t **ppc16Read, const char16_t *pc16ReadEnd){
	const char16_t *pc16ReadNew = *ppc16Read;
	char32_t c32CodePoint = _MCFCRT_DecodeUtf16(&pc16ReadNew, pc16ReadEnd);
	if(!_MCFCRT_UTF_SUCCESS(c32CodePoint)){
		return c32CodePoint;
	}
	c32CodePoint = _MCFCRT_EncodeUtf8(ppchWrite, pchWriteEnd, c32CodePoint);
	if(!_MCFCRT_UTF_SUCCESS(c32CodePoint)){
		return c32CodePoint;
	}
	*ppc16Read = pc16ReadNew;
	return c32CodePoint;
}
char32_t _MCFCRT_EncodeUtf16FromUtf8(char16_t **ppc16Write, char16_t *pc16WriteEnd, const char **ppchRead, const char *pchReadEnd){
	const char *pchRead = *ppchRead;
	char32_t c32CodePoint = _MCFCRT_DecodeUtf8(&pchRead, pchReadEnd);
	if(!_MCFCRT_UTF_SUCCESS(c32CodePoint)){
		return c32CodePoint;
	}
	c32CodePoint = _MCFCRT_EncodeUtf16(ppc16Write, pc16WriteEnd, c32CodePoint);
	if(!_MCFCRT_UTF_SUCCESS(c32CodePoint)){
		return c32CodePoint;
	}
	*ppchRead = pchRead;
	return c32CodePoint;
}

char32_t _MCFCRT_UncheckedDecodeUtf8(const char *restrict *restrict ppchRead){
	register const char *pchRead = *ppchRead;
	register uint_fast32_t u32Temp = (uint8_t)*(pchRead++);
	if(u32Temp < 0x80){ // 7 位
		//
	} else if(u32Temp < 0xC0){ // 孤立的尾部字节
		//
	} else if(u32Temp < 0xE0){ // 11 位
		u32Temp &= 0x1F;
		u32Temp <<= 6;
		uint_fast32_t u32Next = (uint8_t)*(pchRead++);
		u32Temp |= (u32Next & 0x3F);
	} else if(u32Temp < 0xF0){ // 16 位
		u32Temp &= 0x0F;
		u32Temp <<= 12;
		uint_fast32_t u32Next = (uint8_t)*(pchRead++);
		u32Temp |= (u32Next & 0x3F) << 6;
		u32Next = (uint8_t)*(pchRead++);
		u32Temp |= (u32Next & 0x3F);
	} else if(u32Temp < 0xF8){ // 21 位
		u32Temp &= 0x07;
		u32Temp <<= 18;
		uint_fast32_t u32Next = (uint8_t)*(pchRead++);
		u32Temp |= (u32Next & 0x3F) << 12;
		u32Next = (uint8_t)*(pchRead++);
		u32Temp |= (u32Next & 0x3F) << 6;
		u32Next = (uint8_t)*(pchRead++);
		u32Temp |= (u32Next & 0x3F);
	} else {
		//
	}
	*ppchRead = pchRead;
	return (char32_t)u32Temp;
}
char32_t _MCFCRT_UncheckedDecodeUtf16(const char16_t *restrict *restrict ppc16Read){
	register const char16_t *pc16Read = *ppc16Read;
	register uint_fast32_t u32Temp = (uint16_t)*(pc16Read++);
	if(u32Temp < 0xD800){ // 1 编码单元
		//
	} else if(u32Temp < 0xDC00){ // 2 编码单元
		u32Temp &= 0x3FF;
		u32Temp <<= 10;
		uint_fast32_t u32Next = (uint16_t)*(pc16Read++);
		u32Temp |= (u32Next & 0x3FF);
		u32Temp += 0x10000;
	} else if(u32Temp < 0xE000){ // 孤立的低位代理
		//
	} else { // 1 编码单元
		//
	}
	*ppc16Read = pc16Read;
	return (char32_t)u32Temp;
}

char32_t _MCFCRT_UncheckedEncodeUtf8(char *restrict *restrict ppchWrite, char32_t c32CodePoint){
	register char *pchWrite = *ppchWrite;
	register uint_fast32_t u32Temp = (uint32_t)c32CodePoint;
	if(u32Temp < 0x80){ // 7 位
		*(pchWrite++) = (char)u32Temp;
	} else if(u32Temp < 0x800){ // 11 位 = 5 + 6
		*(pchWrite++) = (char)(((u32Temp >>  6) & 0x1F) | 0xC0);
		*(pchWrite++) = (char)(((u32Temp      ) & 0x3F) | 0x80);
	} else if(u32Temp < 0x10000){ // 16 位 = 4 + 6 + 6
		*(pchWrite++) = (char)(((u32Temp >> 12) & 0x0F) | 0xE0);
		*(pchWrite++) = (char)(((u32Temp >>  6) & 0x3F) | 0x80);
		*(pchWrite++) = (char)(((u32Temp      ) & 0x3F) | 0x80);
	} else { // 21 位 = 3 + 6 + 6 + 6
		*(pchWrite++) = (char)(((u32Temp >> 18) & 0x07) | 0xF0);
		*(pchWrite++) = (char)(((u32Temp >> 12) & 0x3F) | 0x80);
		*(pchWrite++) = (char)(((u32Temp >>  6) & 0x3F) | 0x80);
		*(pchWrite++) = (char)(((u32Temp      ) & 0x3F) | 0x80);
	}
	*ppchWrite = pchWrite;
	return c32CodePoint;
}
char32_t _MCFCRT_UncheckedEncodeUtf16(char16_t *restrict *restrict ppc16Write, char32_t c32CodePoint){
	register char16_t *pc16Write = *ppc16Write;
	register uint_fast32_t u32Temp = (uint32_t)c32CodePoint;
	if(u32Temp < 0x10000){ // 1 编码单元
		*(pc16Write++) = (char16_t)u32Temp;
	} else { // 2 编码单元
		u32Temp -= 0x10000;
		*(pc16Write++) = (char16_t)((((u32Temp) >> 10) & 0x03FF) | 0xD800);
		*(pc16Write++) = (char16_t)((((u32Temp)      ) & 0x03FF) | 0xDC00);
	}
	*ppc16Write = pc16Write;
	return c32CodePoint;
}

char32_t _MCFCRT_UncheckedEncodeUtf8FromUtf16(char *restrict *restrict ppchWrite, const char16_t *restrict *restrict ppc16Read){
	char32_t c32CodePoint = _MCFCRT_UncheckedDecodeUtf16(ppc16Read);
	c32CodePoint = _MCFCRT_UncheckedEncodeUtf8(ppchWrite, c32CodePoint);
	return c32CodePoint;
}
char32_t _MCFCRT_UncheckedEncodeUtf16FromUtf8(char16_t *restrict *restrict ppc16Write, const char *restrict *restrict ppchRead){
	char32_t c32CodePoint = _MCFCRT_UncheckedDecodeUtf8(ppchRead);
	c32CodePoint = _MCFCRT_UncheckedEncodeUtf16(ppc16Write, c32CodePoint);
	return c32CodePoint;
}
