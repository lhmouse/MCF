// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "itow.h"
#include "rep_movs.h"

__attribute__((__always_inline__)) static inline wchar_t * Really_itow_u(wchar_t *restrict buffer, uintptr_t value, unsigned min_digits, const wchar_t *restrict table, unsigned radix){
	wchar_t temp[64];
	wchar_t *const end = temp + 64;

	unsigned digits_written = 0;
	// Write digits in reverse order.
	uintptr_t word = value;
	do {
		const unsigned digit_value = (unsigned)(word % radix);
		word /= radix;
		const wchar_t digit = table[digit_value];
		++digits_written;
		*(end - digits_written) = digit;
	} while(word != 0);
	// Pad it with zeroes unless it exceeds the minimum length.
	while(digits_written < min_digits){
		const wchar_t digit = table[0];
		++digits_written;
		*(end - digits_written) = digit;
	}
	// Copy it to the correct location.
	return (wchar_t *)_MCFCRT_rep_movsw(_MCFCRT_NULLPTR, (uint16_t *)buffer, (const uint16_t *)end - digits_written, digits_written);
}

wchar_t * _MCFCRT_itow_d(wchar_t *buffer, intptr_t value){
	return _MCFCRT_itow0d(buffer, value, 0);
}
wchar_t * _MCFCRT_itow0d(wchar_t *buffer, intptr_t value, unsigned min_digits){
	wchar_t *begin = buffer;
	uintptr_t mask = 0;
	if(value < 0){
		*(begin++) = L'-';
		mask = ~mask;
	}
	return Really_itow_u(begin, ((uintptr_t)value ^ mask) - mask, min_digits, L"0123456789", 10);
}

wchar_t * _MCFCRT_itowS_d(wchar_t *buffer, intptr_t value){
	return _MCFCRT_itowS0d(buffer, value, 0);
}
wchar_t * _MCFCRT_itowS0d(wchar_t *buffer, intptr_t value, unsigned min_digits){
	wchar_t *begin = buffer;
	uintptr_t mask = 0;
	if(value < 0){
		*(begin++) = L'-';
		mask = ~mask;
	} else {
		*(begin++) = L'+';
	}
	return Really_itow_u(begin, ((uintptr_t)value ^ mask) - mask, min_digits, L"0123456789", 10);
}

wchar_t * _MCFCRT_itow_u(wchar_t *buffer, uintptr_t value){
	return _MCFCRT_itow0u(buffer, value, 0);
}
wchar_t * _MCFCRT_itow0u(wchar_t *buffer, uintptr_t value, unsigned min_digits){
	return Really_itow_u(buffer, value, min_digits, L"0123456789", 10);
}

wchar_t * _MCFCRT_itow_x(wchar_t *buffer, uintptr_t value){
	return _MCFCRT_itow0x(buffer, value, 0);
}
wchar_t * _MCFCRT_itow0x(wchar_t *buffer, uintptr_t value, unsigned min_digits){
	return Really_itow_u(buffer, value, min_digits, L"0123456789abcdef", 16);
}

wchar_t * _MCFCRT_itow_X(wchar_t *buffer, uintptr_t value){
	return _MCFCRT_itow0X(buffer, value, 0);
}
wchar_t * _MCFCRT_itow0X(wchar_t *buffer, uintptr_t value, unsigned min_digits){
	return Really_itow_u(buffer, value, min_digits, L"0123456789ABCDEF", 16);
}
