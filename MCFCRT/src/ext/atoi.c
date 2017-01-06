// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "atoi.h"

__attribute__((__always_inline__))
static inline char *ReallyAtoiU(uintptr_t *puValue, const char *pchBuffer, unsigned uMaxDigits, uintptr_t uBound, const char *pchTable, unsigned uRadix){
	unsigned uDigitsInput = 0;
	uintptr_t uWord = 0;
	while(uDigitsInput + 1 <= uMaxDigits){
		char chDigit = pchBuffer[uDigitsInput];
		if(('a' <= chDigit) && (chDigit <= 'z')){
			chDigit = (char)(chDigit - 'a' + 'A');
		}
		const char *pchDigitFound = pchTable;
		unsigned uCounter = uRadix;
		__asm__ (
			"xor edx, edx \n"
			"repne scasb \n"
			"setz dl \n"
			"add ecx, edx \n"
			: "+D"(pchDigitFound), "+c"(uCounter)
			: "a"(chDigit)
			: "dx"
		);
		if(uCounter == 0){
			break;
		}
		const unsigned uDigitValue = uRadix - uCounter;
		const uintptr_t uThisBound = (uBound - uDigitValue) / uRadix;
		if(uWord > uThisBound){
			break;
		}
		uWord *= uRadix;
		uWord += uDigitValue;
		++uDigitsInput;
	}
	*puValue = uWord;
	return (char *)pchBuffer + uDigitsInput;
}

char *_MCFCRT_atoi_d(intptr_t *pnValue, const char *pchBuffer){
	return _MCFCRT_atoi0d(pnValue, pchBuffer, UINT_MAX);
}
char *_MCFCRT_atoi_u(uintptr_t *puValue, const char *pchBuffer){
	return _MCFCRT_atoi0u(puValue, pchBuffer, UINT_MAX);
}
char *_MCFCRT_atoi_x(uintptr_t *puValue, const char *pchBuffer){
	return _MCFCRT_atoi0x(puValue, pchBuffer, UINT_MAX);
}
char *_MCFCRT_atoi_X(uintptr_t *puValue, const char *pchBuffer){
	return _MCFCRT_atoi0X(puValue, pchBuffer, UINT_MAX);
}
char *_MCFCRT_atoi0d(intptr_t *pnValue, const char *pchBuffer, unsigned uMaxDigits){
	char *pchEnd;
	if(*pchBuffer == '-'){
		uintptr_t uValue;
		pchEnd = ReallyAtoiU(&uValue, pchBuffer + 1, uMaxDigits, -(uintptr_t)INTPTR_MIN, "0123456789", 10);
		*pnValue = -(intptr_t)uValue;
	} else {
		uintptr_t uValue;
		pchEnd = ReallyAtoiU(&uValue, pchBuffer    , uMaxDigits,  (uintptr_t)INTPTR_MAX, "0123456789", 10);
		*pnValue = (intptr_t)uValue;
	}
	return pchEnd;
}
char *_MCFCRT_atoi0u(uintptr_t *puValue, const char *pchBuffer, unsigned uMaxDigits){
	return ReallyAtoiU(puValue, pchBuffer, uMaxDigits, UINTPTR_MAX, "0123456789"      , 10);
}
char *_MCFCRT_atoi0x(uintptr_t *puValue, const char *pchBuffer, unsigned uMaxDigits){
	return ReallyAtoiU(puValue, pchBuffer, uMaxDigits, UINTPTR_MAX, "0123456789ABCDEF", 16);
}
char *_MCFCRT_atoi0X(uintptr_t *puValue, const char *pchBuffer, unsigned uMaxDigits){
	return ReallyAtoiU(puValue, pchBuffer, uMaxDigits, UINTPTR_MAX, "0123456789ABCDEF", 16);
}
