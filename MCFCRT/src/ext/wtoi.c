// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "wtoi.h"
#include "repnz_scas.h"

__attribute__((__always_inline__))
static inline wchar_t *Really_wtoi_u(_MCFCRT_wtoi_result *peResult, uintptr_t *puValue, const wchar_t *pwcBuffer, unsigned uMaxDigits, uintptr_t uBound, const wchar_t *pwcDualTable, unsigned uRadix){
	unsigned uDigitsRead = 0;
	_MCFCRT_wtoi_result eResult = _MCFCRT_wtoi_result_no_digit;
	// Parse digits.
	uintptr_t uWord = 0;
	while(uDigitsRead + 1 <= uMaxDigits){
		const wchar_t wcDigit = pwcBuffer[uDigitsRead];
		// Search for this digit in the table. Handle lower and upper cases universally.
		uint16_t *pu16DigitInTable;
		if(_MCFCRT_repnz_scasw(&pu16DigitInTable, pwcDualTable, wcDigit, uRadix * 2) == 0){
			break;
		}
		const unsigned uDigitValue = (unsigned)((const wchar_t *)pu16DigitInTable - pwcDualTable) / 2;
		// Check for overflows.
		const uintptr_t uThisBound = (uBound - uDigitValue) / uRadix;
		if(uWord > uThisBound){
			eResult = _MCFCRT_wtoi_result_would_overflow;
			break;
		}
		uWord *= uRadix;
		uWord += uDigitValue;
		++uDigitsRead;
		eResult = _MCFCRT_wtoi_result_success;
	}
	*peResult = eResult;
	*puValue = uWord;
	return (wchar_t *)pwcBuffer + uDigitsRead;
}

wchar_t *_MCFCRT_wtoi_d(_MCFCRT_wtoi_result *peResult, intptr_t *pnValue, const wchar_t *pwcBuffer){
	return _MCFCRT_wtoi0d(peResult, pnValue, pwcBuffer, UINT_MAX);
}
wchar_t *_MCFCRT_wtoi_u(_MCFCRT_wtoi_result *peResult, uintptr_t *puValue, const wchar_t *pwcBuffer){
	return _MCFCRT_wtoi0u(peResult, puValue, pwcBuffer, UINT_MAX);
}
wchar_t *_MCFCRT_wtoi_x(_MCFCRT_wtoi_result *peResult, uintptr_t *puValue, const wchar_t *pwcBuffer){
	return _MCFCRT_wtoi0x(peResult, puValue, pwcBuffer, UINT_MAX);
}
wchar_t *_MCFCRT_wtoi_X(_MCFCRT_wtoi_result *peResult, uintptr_t *puValue, const wchar_t *pwcBuffer){
	return _MCFCRT_wtoi0X(peResult, puValue, pwcBuffer, UINT_MAX);
}
wchar_t *_MCFCRT_wtoi0d(_MCFCRT_wtoi_result *peResult, intptr_t *pnValue, const wchar_t *pwcBuffer, unsigned uMaxDigits){
	wchar_t *pwcEnd;
	if(*pwcBuffer == L'-'){
		uintptr_t uValue;
		pwcEnd = Really_wtoi_u(peResult, &uValue, pwcBuffer + 1, uMaxDigits, -(uintptr_t)INTPTR_MIN, L"0123456789", 10);
		*pnValue = -(intptr_t)uValue;
	} else {
		uintptr_t uValue;
		pwcEnd = Really_wtoi_u(peResult, &uValue, pwcBuffer    , uMaxDigits,  (uintptr_t)INTPTR_MAX, L"0123456789", 10);
		*pnValue =  (intptr_t)uValue;
	}
	return pwcEnd;
}
wchar_t *_MCFCRT_wtoi0u(_MCFCRT_wtoi_result *peResult, uintptr_t *puValue, const wchar_t *pwcBuffer, unsigned uMaxDigits){
	return Really_wtoi_u(peResult, puValue, pwcBuffer, uMaxDigits, UINTPTR_MAX, L"0123456789"      , 10);
}
wchar_t *_MCFCRT_wtoi0x(_MCFCRT_wtoi_result *peResult, uintptr_t *puValue, const wchar_t *pwcBuffer, unsigned uMaxDigits){
	return Really_wtoi_u(peResult, puValue, pwcBuffer, uMaxDigits, UINTPTR_MAX, L"0123456789ABCDEF", 16);
}
wchar_t *_MCFCRT_wtoi0X(_MCFCRT_wtoi_result *peResult, uintptr_t *puValue, const wchar_t *pwcBuffer, unsigned uMaxDigits){
	return Really_wtoi_u(peResult, puValue, pwcBuffer, uMaxDigits, UINTPTR_MAX, L"0123456789ABCDEF", 16);
}
