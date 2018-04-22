// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "wtoi.h"
#include "rep_scas.h"

__attribute__((__always_inline__))
static inline wchar_t *Really_wtoi_u(_MCFCRT_wtoi_result *restrict result_out, uintptr_t *restrict value_out, const wchar_t *restrict buffer, unsigned max_digits, uintptr_t bound, const wchar_t *restrict dual_table, unsigned radix){
	unsigned digits_read = 0;
	_MCFCRT_wtoi_result result = _MCFCRT_wtoi_result_no_digit;
	// Parse digits.
	uintptr_t word = 0;
	while(digits_read + 1 <= max_digits){
		const wchar_t digit = buffer[digits_read];
		// Search for this digit in the table. Handle lower and upper cases universally.
		uint16_t *digit_in_table;
		if(_MCFCRT_repnz_scasw(&digit_in_table, (const uint16_t *)dual_table, (uint16_t)digit, radix * 2) != 0){
			break;
		}
		const unsigned digit_value = (unsigned)((const wchar_t *)digit_in_table - dual_table) / 2;
		// Check for overflow.
		const uintptr_t digit_bound = (bound - digit_value) / radix;
		if(word > digit_bound){
			result = _MCFCRT_wtoi_result_would_overflow;
			break;
		}
		word *= radix;
		word += digit_value;
		++digits_read;
		result = _MCFCRT_wtoi_result_success;
	}
	*result_out = result;
	*value_out = word;
	return (wchar_t *)buffer + digits_read;
}

wchar_t *_MCFCRT_wtoi_d(_MCFCRT_wtoi_result *restrict result_out, intptr_t *restrict value_out, const wchar_t *restrict buffer){
	return _MCFCRT_wtoi0d(result_out, value_out, buffer, UINT_MAX);
}
wchar_t *_MCFCRT_wtoi0d(_MCFCRT_wtoi_result *restrict result_out, intptr_t *restrict value_out, const wchar_t *restrict buffer, unsigned max_digits){
	const wchar_t *begin = buffer;
	uintptr_t mask = 0;
	if(*begin == L'-'){
		++begin;
		mask = ~mask;
	} else if(*begin == L'+'){
		++begin;
	}
	uintptr_t abs;
	wchar_t *end = Really_wtoi_u(result_out, &abs, begin, max_digits, INTPTR_MAX ^ mask, L"00112233445566778899", 10);
	*value_out = (intptr_t)((abs ^ mask) - mask);
	return end;
}

wchar_t *_MCFCRT_wtoi_u(_MCFCRT_wtoi_result *restrict result_out, uintptr_t *restrict value_out, const wchar_t *restrict buffer){
	return _MCFCRT_wtoi0u(result_out, value_out, buffer, UINT_MAX);
}
wchar_t *_MCFCRT_wtoi0u(_MCFCRT_wtoi_result *restrict result_out, uintptr_t *restrict value_out, const wchar_t *restrict buffer, unsigned max_digits){
	return Really_wtoi_u(result_out, value_out, buffer, max_digits, UINTPTR_MAX, L"00112233445566778899", 10);
}

wchar_t *_MCFCRT_wtoi_x(_MCFCRT_wtoi_result *restrict result_out, uintptr_t *restrict value_out, const wchar_t *restrict buffer){
	return _MCFCRT_wtoi0x(result_out, value_out, buffer, UINT_MAX);
}
wchar_t *_MCFCRT_wtoi0x(_MCFCRT_wtoi_result *restrict result_out, uintptr_t *restrict value_out, const wchar_t *restrict buffer, unsigned max_digits){
	return Really_wtoi_u(result_out, value_out, buffer, max_digits, UINTPTR_MAX, L"00112233445566778899aAbBcCdDeEfF", 16);
}

wchar_t *_MCFCRT_wtoi_X(_MCFCRT_wtoi_result *restrict result_out, uintptr_t *restrict value_out, const wchar_t *restrict buffer){
	return _MCFCRT_wtoi0X(result_out, value_out, buffer, UINT_MAX);
}
wchar_t *_MCFCRT_wtoi0X(_MCFCRT_wtoi_result *restrict result_out, uintptr_t *restrict value_out, const wchar_t *restrict buffer, unsigned max_digits){
	return Really_wtoi_u(result_out, value_out, buffer, max_digits, UINTPTR_MAX, L"00112233445566778899aAbBcCdDeEfF", 16);
}
