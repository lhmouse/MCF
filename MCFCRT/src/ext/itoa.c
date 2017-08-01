// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "itoa.h"
#include "rep_movs.h"

__attribute__((__always_inline__))
static inline char *Really_itoa_u(char *restrict buffer, uintptr_t value, unsigned min_digits, const char *restrict table, unsigned radix){
	char temp[64];
	char *const end = temp + 64;

	unsigned digits_written = 0;
	// Write digits in reverse order.
	uintptr_t word = value;
	do {
		const unsigned digit_value = (unsigned)(word % radix);
		word /= radix;
		const char digit = table[digit_value];
		++digits_written;
		*(end - digits_written) = digit;
	} while(word != 0);
	// Pad it with zeroes unless it exceeds the minimum length.
	while(digits_written < min_digits){
		const char digit = table[0];
		++digits_written;
		*(end - digits_written) = digit;
	}
	// Copy it to the correct location.
	return _MCFCRT_rep_movsb(_MCFCRT_NULLPTR, buffer, (const uint8_t *)end - digits_written, digits_written);
}

char *_MCFCRT_itoa_d(char *buffer, intptr_t value){
	return _MCFCRT_itoa0d(buffer, value, 0);
}
char *_MCFCRT_itoa_u(char *buffer, uintptr_t value){
	return _MCFCRT_itoa0u(buffer, value, 0);
}
char *_MCFCRT_itoa_x(char *buffer, uintptr_t value){
	return _MCFCRT_itoa0x(buffer, value, 0);
}
char *_MCFCRT_itoa_X(char *buffer, uintptr_t value){
	return _MCFCRT_itoa0X(buffer, value, 0);
}
char *_MCFCRT_itoa0d(char *buffer, intptr_t value, unsigned min_digits){
	char *end;
	if(value < 0){
		*buffer = '-';
		end = Really_itoa_u(buffer + 1, (uintptr_t)-value, min_digits, "0123456789", 10);
	} else {
		end = Really_itoa_u(buffer + 0, (uintptr_t) value, min_digits, "0123456789", 10);
	}
	return end;
}
char *_MCFCRT_itoa0u(char *buffer, uintptr_t value, unsigned min_digits){
	return Really_itoa_u(buffer, value, min_digits, "0123456789"      , 10);
}
char *_MCFCRT_itoa0x(char *buffer, uintptr_t value, unsigned min_digits){
	return Really_itoa_u(buffer, value, min_digits, "0123456789abcdef", 16);
}
char *_MCFCRT_itoa0X(char *buffer, uintptr_t value, unsigned min_digits){
	return Really_itoa_u(buffer, value, min_digits, "0123456789ABCDEF", 16);
}
