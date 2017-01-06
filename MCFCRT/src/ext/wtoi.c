// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "wtoi.h"

__attribute__((__always_inline__))
static inline wchar_t *ReallyWtoiU(uintptr_t *puValue, const wchar_t *pwcBuffer, unsigned uMaxDigits, uintptr_t uBound, const wchar_t *pwcTable, unsigned uRadix){
	unsigned uDigitsInput = 0;
	uintptr_t uWord = 0;
	while(uDigitsInput + 1 <= uMaxDigits){
		wchar_t wcDigit = pwcBuffer[uDigitsInput];
		if((L'a' <= wcDigit) && (wcDigit <= L'z')){
			wcDigit = (wchar_t)(wcDigit - L'a' + L'A');
		}
		const wchar_t *pwcDigitFound = pwcTable;
		unsigned uCounter = uRadix;
		__asm__ (
			"xor edx, edx \n"
			"repne scasw \n"
			"setz dl \n"
			"add ecx, edx \n"
			: "+D"(pwcDigitFound), "+c"(uCounter)
			: "a"(wcDigit)
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
	return (wchar_t *)pwcBuffer + uDigitsInput;
}

wchar_t *_MCFCRT_wtoi_d(intptr_t *pnValue, const wchar_t *pwcBuffer){
	return _MCFCRT_wtoi0d(pnValue, pwcBuffer, UINT_MAX);
}
wchar_t *_MCFCRT_wtoi_u(uintptr_t *puValue, const wchar_t *pwcBuffer){
	return _MCFCRT_wtoi0u(puValue, pwcBuffer, UINT_MAX);
}
wchar_t *_MCFCRT_wtoi_x(uintptr_t *puValue, const wchar_t *pwcBuffer){
	return _MCFCRT_wtoi0x(puValue, pwcBuffer, UINT_MAX);
}
wchar_t *_MCFCRT_wtoi_X(uintptr_t *puValue, const wchar_t *pwcBuffer){
	return _MCFCRT_wtoi0X(puValue, pwcBuffer, UINT_MAX);
}
wchar_t *_MCFCRT_wtoi0d(intptr_t *pnValue, const wchar_t *pwcBuffer, unsigned uMaxDigits){
	wchar_t *pwcEnd;
	if(*pwcBuffer == '-'){
		uintptr_t uValue;
		pwcEnd = ReallyWtoiU(&uValue, pwcBuffer + 1, uMaxDigits, -(uintptr_t)INTPTR_MIN, L"0123456789", 10);
		*pnValue = -(intptr_t)uValue;
	} else {
		uintptr_t uValue;
		pwcEnd = ReallyWtoiU(&uValue, pwcBuffer    , uMaxDigits,  (uintptr_t)INTPTR_MAX, L"0123456789", 10);
		*pnValue = (intptr_t)uValue;
	}
	return pwcEnd;
}
wchar_t *_MCFCRT_wtoi0u(uintptr_t *puValue, const wchar_t *pwcBuffer, unsigned uMaxDigits){
	return ReallyWtoiU(puValue, pwcBuffer, uMaxDigits, UINTPTR_MAX, L"0123456789"      , 10);
}
wchar_t *_MCFCRT_wtoi0x(uintptr_t *puValue, const wchar_t *pwcBuffer, unsigned uMaxDigits){
	return ReallyWtoiU(puValue, pwcBuffer, uMaxDigits, UINTPTR_MAX, L"0123456789ABCDEF", 16);
}
wchar_t *_MCFCRT_wtoi0X(uintptr_t *puValue, const wchar_t *pwcBuffer, unsigned uMaxDigits){
	return ReallyWtoiU(puValue, pwcBuffer, uMaxDigits, UINTPTR_MAX, L"0123456789ABCDEF", 16);
}
