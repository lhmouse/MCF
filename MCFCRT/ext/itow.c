// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "itow.h"

static inline wchar_t *ReallyItowU(wchar_t *pwcBuffer, uintptr_t uValue, unsigned uMinDigits, const wchar_t *pwcTable, unsigned uRadix){
	unsigned uDigitsOutput = 0;
	do {
		const wchar_t wcDigit = pwcTable[uValue % uRadix];
		uValue /= uRadix;
		pwcBuffer[uDigitsOutput++] = wcDigit;
	} while(uValue > 0);

	while(uDigitsOutput < uMinDigits){
		pwcBuffer[uDigitsOutput++] = pwcTable[0];
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
	if(nValue < 0){
		*(pwcBuffer++) = '-';
		nValue = -nValue;
	}
	return _MCFCRT_itow0u(pwcBuffer, (uintptr_t)nValue, uMinDigits);
}
wchar_t *_MCFCRT_itow0u(wchar_t *pwcBuffer, uintptr_t uValue, unsigned uMinDigits){
	return ReallyItowU(pwcBuffer, uValue, uMinDigits, L"0123456789", 10);
}
wchar_t *_MCFCRT_itow0x(wchar_t *pwcBuffer, uintptr_t uValue, unsigned uMinDigits){
	return ReallyItowU(pwcBuffer, uValue, uMinDigits, L"0123456789abcdef", 16);
}
wchar_t *_MCFCRT_itow0X(wchar_t *pwcBuffer, uintptr_t uValue, unsigned uMinDigits){
	return ReallyItowU(pwcBuffer, uValue, uMinDigits, L"0123456789ABCDEF", 16);
}
