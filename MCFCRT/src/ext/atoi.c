// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "atoi.h"
#include "repnz_scas.h"

__attribute__((__always_inline__))
static inline char *Really_atoi_u(_MCFCRT_atoi_result *peResult, uintptr_t *puValue, const char *pchBuffer, unsigned uMaxDigits, uintptr_t uBound, const char *pchDualTable, unsigned uRadix){
	unsigned uDigitsRead = 0;
	_MCFCRT_atoi_result eResult = _MCFCRT_atoi_result_no_digit;
	// Parse digits.
	uintptr_t uWord = 0;
	while(uDigitsRead + 1 <= uMaxDigits){
		const char chDigit = pchBuffer[uDigitsRead];
		// Search for this digit in the table. Handle lower and upper cases universally.
		void *pDigitInTable;
		if(_MCFCRT_repnz_scasb(&pDigitInTable, pchDualTable, chDigit, uRadix * 2) != 0){
			break;
		}
		const unsigned uDigitValue = (unsigned)((const char *)pDigitInTable - pchDualTable) / 2;
		// Check for overflows.
		const uintptr_t uThisBound = (uBound - uDigitValue) / uRadix;
		if(uWord > uThisBound){
			eResult = _MCFCRT_atoi_result_would_overflow;
			break;
		}
		uWord *= uRadix;
		uWord += uDigitValue;
		++uDigitsRead;
		eResult = _MCFCRT_atoi_result_success;
	}
	*peResult = eResult;
	*puValue = uWord;
	return (char *)pchBuffer + uDigitsRead;
}

char *_MCFCRT_atoi_d(_MCFCRT_atoi_result *peResult, intptr_t *pnValue, const char *pchBuffer){
	return _MCFCRT_atoi0d(peResult, pnValue, pchBuffer, UINT_MAX);
}
char *_MCFCRT_atoi_u(_MCFCRT_atoi_result *peResult, uintptr_t *puValue, const char *pchBuffer){
	return _MCFCRT_atoi0u(peResult, puValue, pchBuffer, UINT_MAX);
}
char *_MCFCRT_atoi_x(_MCFCRT_atoi_result *peResult, uintptr_t *puValue, const char *pchBuffer){
	return _MCFCRT_atoi0x(peResult, puValue, pchBuffer, UINT_MAX);
}
char *_MCFCRT_atoi_X(_MCFCRT_atoi_result *peResult, uintptr_t *puValue, const char *pchBuffer){
	return _MCFCRT_atoi0X(peResult, puValue, pchBuffer, UINT_MAX);
}
char *_MCFCRT_atoi0d(_MCFCRT_atoi_result *peResult, intptr_t *pnValue, const char *pchBuffer, unsigned uMaxDigits){
	char *pchEnd;
	if(*pchBuffer == '-'){
		uintptr_t uValue;
		pchEnd = Really_atoi_u(peResult, &uValue, pchBuffer + 1, uMaxDigits, -(uintptr_t)INTPTR_MIN, "00112233445566778899", 10);
		*pnValue = -(intptr_t)uValue;
	} else {
		uintptr_t uValue;
		pchEnd = Really_atoi_u(peResult, &uValue, pchBuffer    , uMaxDigits,  (uintptr_t)INTPTR_MAX, "00112233445566778899", 10);
		*pnValue =  (intptr_t)uValue;
	}
	return pchEnd;
}
char *_MCFCRT_atoi0u(_MCFCRT_atoi_result *peResult, uintptr_t *puValue, const char *pchBuffer, unsigned uMaxDigits){
	return Really_atoi_u(peResult, puValue, pchBuffer, uMaxDigits, UINTPTR_MAX, "00112233445566778899"            , 10);
}
char *_MCFCRT_atoi0x(_MCFCRT_atoi_result *peResult, uintptr_t *puValue, const char *pchBuffer, unsigned uMaxDigits){
	return Really_atoi_u(peResult, puValue, pchBuffer, uMaxDigits, UINTPTR_MAX, "00112233445566778899aAbBcCdDeEfF", 16);
}
char *_MCFCRT_atoi0X(_MCFCRT_atoi_result *peResult, uintptr_t *puValue, const char *pchBuffer, unsigned uMaxDigits){
	return Really_atoi_u(peResult, puValue, pchBuffer, uMaxDigits, UINTPTR_MAX, "00112233445566778899aAbBcCdDeEfF", 16);
}
