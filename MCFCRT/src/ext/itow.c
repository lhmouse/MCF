// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "itow.h"

__attribute__((__always_inline__))
static inline wchar_t *ReallyItowU(wchar_t *pwcBuffer, uintptr_t uValue, unsigned uMinDigits, const wchar_t *pwcTable, unsigned uRadix){
	unsigned uDigitsOutput = 0;
	uintptr_t uWord = uValue;
	do {
		const unsigned uDigitValue = (unsigned)(uWord % uRadix);
		uWord /= uRadix;
		const wchar_t wcDigit = pwcTable[uDigitValue];
		pwcBuffer[uDigitsOutput] = wcDigit;
		++uDigitsOutput;
	} while(uWord > 0);
	while(uDigitsOutput + 1 <= uMinDigits){
		pwcBuffer[uDigitsOutput] = pwcTable[0];
		++uDigitsOutput;
	}
	for(unsigned i = 0, j = uDigitsOutput - 1; i < j; ++i, --j){
		wchar_t wc = pwcBuffer[i];
		pwcBuffer[i] = pwcBuffer[j];
		pwcBuffer[j] = wc;
	}
	return pwcBuffer + uDigitsOutput;
}

wchar_t *_MCFCRT_itow_d(wchar_t *pwcBuffer, intptr_t nValue){
	return _MCFCRT_itow0d(pwcBuffer, nValue, 0);
}
wchar_t *_MCFCRT_itow_u(wchar_t *pwcBuffer, uintptr_t uValue){
	return _MCFCRT_itow0u(pwcBuffer, uValue, 0);
}
wchar_t *_MCFCRT_itow_x(wchar_t *pwcBuffer, uintptr_t uValue){
	return _MCFCRT_itow0x(pwcBuffer, uValue, 0);
}
wchar_t *_MCFCRT_itow_X(wchar_t *pwcBuffer, uintptr_t uValue){
	return _MCFCRT_itow0X(pwcBuffer, uValue, 0);
}
wchar_t *_MCFCRT_itow0d(wchar_t *pwcBuffer, intptr_t nValue, unsigned uMinDigits){
	wchar_t *pwcEnd;
	if(nValue < 0){
		*pwcBuffer = L'-';
		pwcEnd = ReallyItowU(pwcBuffer + 1, (uintptr_t)-nValue, uMinDigits, L"0123456789", 10);
	} else {
		pwcEnd = ReallyItowU(pwcBuffer    , (uintptr_t) nValue, uMinDigits, L"0123456789", 10);
	}
	return pwcEnd;
}
wchar_t *_MCFCRT_itow0u(wchar_t *pwcBuffer, uintptr_t uValue, unsigned uMinDigits){
	return ReallyItowU(pwcBuffer, uValue, uMinDigits, L"0123456789"      , 10);
}
wchar_t *_MCFCRT_itow0x(wchar_t *pwcBuffer, uintptr_t uValue, unsigned uMinDigits){
	return ReallyItowU(pwcBuffer, uValue, uMinDigits, L"0123456789abcdef", 16);
}
wchar_t *_MCFCRT_itow0X(wchar_t *pwcBuffer, uintptr_t uValue, unsigned uMinDigits){
	return ReallyItowU(pwcBuffer, uValue, uMinDigits, L"0123456789ABCDEF", 16);
}
