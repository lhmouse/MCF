// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "itow.h"
#include "rep_movs.h"

__attribute__((__always_inline__))
static inline wchar_t *Really_itow_u(wchar_t *restrict pwcBuffer, uintptr_t uValue, unsigned uMinDigits, const wchar_t *restrict pwcTable, unsigned uRadix){
	wchar_t achTemp[64];
	wchar_t *const pwcTempEnd = achTemp + 64;

	unsigned uDigitsWritten = 0;
	// Write digits in reverse order.
	uintptr_t uWord = uValue;
	do {
		const unsigned uDigitValue = (unsigned)(uWord % uRadix);
		uWord /= uRadix;
		const wchar_t wcDigit = pwcTable[uDigitValue];
		++uDigitsWritten;
		*(pwcTempEnd - uDigitsWritten) = wcDigit;
	} while(uWord != 0);
	// Pad it with zeroes unless it exceeds the minimum length.
	while(uDigitsWritten < uMinDigits){
		const wchar_t wcDigit = pwcTable[0];
		++uDigitsWritten;
		*(pwcTempEnd - uDigitsWritten) = wcDigit;
	}
	// Copy it to the correct location.
	return (wchar_t *)_MCFCRT_rep_movsw((uint16_t *)pwcBuffer, (const uint16_t *)pwcTempEnd - uDigitsWritten, uDigitsWritten);
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
		pwcEnd = Really_itow_u(pwcBuffer + 1, (uintptr_t)-nValue, uMinDigits, L"0123456789", 10);
	} else {
		pwcEnd = Really_itow_u(pwcBuffer    , (uintptr_t) nValue, uMinDigits, L"0123456789", 10);
	}
	return pwcEnd;
}
wchar_t *_MCFCRT_itow0u(wchar_t *pwcBuffer, uintptr_t uValue, unsigned uMinDigits){
	return Really_itow_u(pwcBuffer, uValue, uMinDigits, L"0123456789"      , 10);
}
wchar_t *_MCFCRT_itow0x(wchar_t *pwcBuffer, uintptr_t uValue, unsigned uMinDigits){
	return Really_itow_u(pwcBuffer, uValue, uMinDigits, L"0123456789abcdef", 16);
}
wchar_t *_MCFCRT_itow0X(wchar_t *pwcBuffer, uintptr_t uValue, unsigned uMinDigits){
	return Really_itow_u(pwcBuffer, uValue, uMinDigits, L"0123456789ABCDEF", 16);
}
