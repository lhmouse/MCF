// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "itoa.h"

static inline char *ReallyItoaU(char *pchBuffer, uintptr_t uValue, unsigned uMinDigits, const char *pchTable, unsigned uRadix){
	unsigned uDigitsOutput = 0;
	do {
		const char chDigit = pchTable[uValue % uRadix];
		uValue /= uRadix;
		pchBuffer[uDigitsOutput++] = chDigit;
	} while(uValue > 0);

	while(uDigitsOutput < uMinDigits){
		pchBuffer[uDigitsOutput++] = pchTable[0];
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
	if(nValue < 0){
		*(pchBuffer++) = '-';
		nValue = -nValue;
	}
	return _MCFCRT_itoa0u(pchBuffer, (uintptr_t)nValue, uMinDigits);
}
char *_MCFCRT_itoa0u(char *pchBuffer, uintptr_t uValue, unsigned uMinDigits){
	return ReallyItoaU(pchBuffer, uValue, uMinDigits, "0123456789", 10);
}
char *_MCFCRT_itoa0x(char *pchBuffer, uintptr_t uValue, unsigned uMinDigits){
	return ReallyItoaU(pchBuffer, uValue, uMinDigits, "0123456789abcdef", 16);
}
char *_MCFCRT_itoa0X(char *pchBuffer, uintptr_t uValue, unsigned uMinDigits){
	return ReallyItoaU(pchBuffer, uValue, uMinDigits, "0123456789ABCDEF", 16);
}
