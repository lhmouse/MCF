// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "atoi.h"
#include "rep_scas.h"

__attribute__((__always_inline__))
static inline char *Really_atoi_u(_MCFCRT_atoi_result *restrict result_out, uintptr_t *restrict value_out, const char *restrict buffer, unsigned max_digits, uintptr_t bound, const char *restrict dual_table, unsigned radix){
	unsigned digits_read = 0;
	_MCFCRT_atoi_result result = _MCFCRT_atoi_result_no_digit;
	// Parse digits.
	uintptr_t word = 0;
	while(digits_read + 1 <= max_digits){
		const char digit = buffer[digits_read];
		// Search for this digit in the table. Handle lower and upper cases universally.
		uint8_t *digit_in_table;
		if(_MCFCRT_repnz_scasb(&digit_in_table, (const uint8_t *)dual_table, (uint8_t)digit, radix * 2) != 0){
			break;
		}
		const unsigned digit_value = (unsigned)((const char *)digit_in_table - dual_table) / 2;
		// Check for overflow.
		const uintptr_t digit_bound = (bound - digit_value) / radix;
		if(word > digit_bound){
			result = _MCFCRT_atoi_result_would_overflow;
			break;
		}
		word *= radix;
		word += digit_value;
		++digits_read;
		result = _MCFCRT_atoi_result_success;
	}
	*result_out = result;
	*value_out = word;
	return (char *)buffer + digits_read;
}

char *_MCFCRT_atoi_d(_MCFCRT_atoi_result *restrict result_out, intptr_t *restrict value_out, const char *restrict buffer){
	return _MCFCRT_atoi0d(result_out, value_out, buffer, UINT_MAX);
}
char *_MCFCRT_atoi_u(_MCFCRT_atoi_result *restrict result_out, uintptr_t *restrict value_out, const char *restrict buffer){
	return _MCFCRT_atoi0u(result_out, value_out, buffer, UINT_MAX);
}
char *_MCFCRT_atoi_x(_MCFCRT_atoi_result *restrict result_out, uintptr_t *restrict value_out, const char *restrict buffer){
	return _MCFCRT_atoi0x(result_out, value_out, buffer, UINT_MAX);
}
char *_MCFCRT_atoi_X(_MCFCRT_atoi_result *restrict result_out, uintptr_t *restrict value_out, const char *restrict buffer){
	return _MCFCRT_atoi0X(result_out, value_out, buffer, UINT_MAX);
}
char *_MCFCRT_atoi0d(_MCFCRT_atoi_result *restrict result_out, intptr_t *restrict value_out, const char *restrict buffer, unsigned max_digits){
	char *end;
	if(*buffer == '-'){
		uintptr_t value;
		end = Really_atoi_u(result_out, &value, buffer + 1, max_digits, -(uintptr_t)INTPTR_MIN, "00112233445566778899", 10);
		*value_out = -(intptr_t)value;
	} else {
		uintptr_t value;
		end = Really_atoi_u(result_out, &value, buffer    , max_digits,  (uintptr_t)INTPTR_MAX, "00112233445566778899", 10);
		*value_out =  (intptr_t)value;
	}
	return end;
}
char *_MCFCRT_atoi0u(_MCFCRT_atoi_result *restrict result_out, uintptr_t *restrict value_out, const char *restrict buffer, unsigned max_digits){
	return Really_atoi_u(result_out, value_out, buffer, max_digits, UINTPTR_MAX, "00112233445566778899"            , 10);
}
char *_MCFCRT_atoi0x(_MCFCRT_atoi_result *restrict result_out, uintptr_t *restrict value_out, const char *restrict buffer, unsigned max_digits){
	return Really_atoi_u(result_out, value_out, buffer, max_digits, UINTPTR_MAX, "00112233445566778899aAbBcCdDeEfF", 16);
}
char *_MCFCRT_atoi0X(_MCFCRT_atoi_result *restrict result_out, uintptr_t *restrict value_out, const char *restrict buffer, unsigned max_digits){
	return Really_atoi_u(result_out, value_out, buffer, max_digits, UINTPTR_MAX, "00112233445566778899aAbBcCdDeEfF", 16);
}
