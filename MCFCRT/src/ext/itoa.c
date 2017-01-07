// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "itoa.h"
#include "rep_movs.h"

__attribute__((__always_inline__))
static inline char *Really_itoa_u(char *restrict pchBuffer, uintptr_t uValue, unsigned uMinDigits, const char *restrict pchTable, unsigned uRadix){
	char achTemp[64];
	char *const pchTempEnd = achTemp + 64;

	unsigned uDigitsWritten = 0;
	// Write digits in reverse order.
	uintptr_t uWord = uValue;
	do {
		const unsigned uDigitValue = (unsigned)(uWord % uRadix);
		uWord /= uRadix;
		const char chDigit = pchTable[uDigitValue];
		++uDigitsWritten;
		*(pchTempEnd - uDigitsWritten) = chDigit;
	} while(uWord != 0);
	// Pad it with zeroes unless it exceeds the minimum length.
	while(uDigitsWritten < uMinDigits){
		const char chDigit = pchTable[0];
		++uDigitsWritten;
		*(pchTempEnd - uDigitsWritten) = chDigit;
	}
	// Copy it to the correct location.
	return _MCFCRT_rep_movsb(pchBuffer, pchTempEnd - uDigitsWritten, uDigitsWritten);
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
		pchEnd = Really_itoa_u(pchBuffer + 1, (uintptr_t)-nValue, uMinDigits, "0123456789", 10);
	} else {
		pchEnd = Really_itoa_u(pchBuffer    , (uintptr_t) nValue, uMinDigits, "0123456789", 10);
	}
	return pchEnd;
}
char *_MCFCRT_itoa0u(char *pchBuffer, uintptr_t uValue, unsigned uMinDigits){
	return Really_itoa_u(pchBuffer, uValue, uMinDigits, "0123456789"      , 10);
}
char *_MCFCRT_itoa0x(char *pchBuffer, uintptr_t uValue, unsigned uMinDigits){
	return Really_itoa_u(pchBuffer, uValue, uMinDigits, "0123456789abcdef", 16);
}
char *_MCFCRT_itoa0X(char *pchBuffer, uintptr_t uValue, unsigned uMinDigits){
	return Really_itoa_u(pchBuffer, uValue, uMinDigits, "0123456789ABCDEF", 16);
}
