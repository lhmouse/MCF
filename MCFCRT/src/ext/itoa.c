// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "itoa.h"

__attribute__((__always_inline__))
static inline char *ReallyItoaU(char *pchBuffer, uintptr_t uValue, unsigned uMinDigits, const char *pchTable, unsigned uRadix){
	unsigned uDigitsOutput = 0;
	uintptr_t uWord = uValue;
	do {
		const unsigned uDigitValue = (unsigned)(uWord % uRadix);
		uWord /= uRadix;
		const char chDigit = pchTable[uDigitValue];
		pchBuffer[uDigitsOutput] = chDigit;
		++uDigitsOutput;
	} while(uWord > 0);
	while(uDigitsOutput + 1 <= uMinDigits){
		pchBuffer[uDigitsOutput] = pchTable[0];
		++uDigitsOutput;
	}
	for(unsigned i = 0, j = uDigitsOutput - 1; i < j; ++i, --j){
		char ch = pchBuffer[i];
		pchBuffer[i] = pchBuffer[j];
		pchBuffer[j] = ch;
	}
	return pchBuffer + uDigitsOutput;
}

char *_MCFCRT_itoa_d(char *pchBuffer, intptr_t nValue){
	return _MCFCRT_itoa0d(pchBuffer, nValue, 0);
}
char *_MCFCRT_itoa_u(char *pchBuffer, uintptr_t uValue){
	return _MCFCRT_itoa0u(pchBuffer, uValue, 0);
}
char *_MCFCRT_itoa_x(char *pchBuffer, uintptr_t uValue){
	return _MCFCRT_itoa0x(pchBuffer, uValue, 0);
}
char *_MCFCRT_itoa_X(char *pchBuffer, uintptr_t uValue){
	return _MCFCRT_itoa0X(pchBuffer, uValue, 0);
}
char *_MCFCRT_itoa0d(char *pchBuffer, intptr_t nValue, unsigned uMinDigits){
	char *pchEnd;
	if(nValue < 0){
		*pchBuffer = '-';
		pchEnd = ReallyItoaU(pchBuffer + 1, (uintptr_t)-nValue, uMinDigits, "0123456789", 10);
	} else {
		pchEnd = ReallyItoaU(pchBuffer    , (uintptr_t) nValue, uMinDigits, "0123456789", 10);
	}
	return pchEnd;
}
char *_MCFCRT_itoa0u(char *pchBuffer, uintptr_t uValue, unsigned uMinDigits){
	return ReallyItoaU(pchBuffer, uValue, uMinDigits, "0123456789"      , 10);
}
char *_MCFCRT_itoa0x(char *pchBuffer, uintptr_t uValue, unsigned uMinDigits){
	return ReallyItoaU(pchBuffer, uValue, uMinDigits, "0123456789abcdef", 16);
}
char *_MCFCRT_itoa0X(char *pchBuffer, uintptr_t uValue, unsigned uMinDigits){
	return ReallyItoaU(pchBuffer, uValue, uMinDigits, "0123456789ABCDEF", 16);
}
