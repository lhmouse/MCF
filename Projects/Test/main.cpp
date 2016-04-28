#include <MCF/StdMCF.hpp>
#include <MCFCRT/ext/utf.h>
#include <cstdio>
#include <MCF/Thread/Monitor.hpp>

extern "C" unsigned _MCFCRT_Main(){
	constexpr char     src8 [] = u8"abcΑΒΒ喵喵𤭢";
	constexpr char16_t src16[] =  u"abcΑΒΒ喵喵𤭢";
	constexpr char32_t src32[] =  U"abcΑΒΒ喵喵𤭢";

	char     dst8 [100];
	char16_t dst16[100];
	char32_t dst32[100];

	unsigned long    c;
	const char     * r8 ;
	      char     * w8 ;
	const char16_t * r16;
	      char16_t * w16;
	const char32_t * r32;
	      char32_t * w32;

	std::memset(dst32, -1, sizeof(dst32));
	w32 = dst32;
	r8 = src8;
	do {
		c = ::_MCFCRT_DecodeUtf8(&r8);
		*(w32++) = c;
	} while(c);
	std::printf("utf8  -> utf32, len = %td, cmp = %d\n", w32 - dst32, std::memcmp(src32, dst32, sizeof(src32)));

	std::memset(dst32, -1, sizeof(dst32));
	w32 = dst32;
	r16 = src16;
	do {
		c = ::_MCFCRT_DecodeUtf16(&r16);
		*(w32++) = c;
	} while(c);
	std::printf("utf16 -> utf32, len = %td, cmp = %d\n", w32 - dst32, std::memcmp(src32, dst32, sizeof(src32)));

	std::memset(dst8, -1, sizeof(dst8));
	w8 = dst8;
	r32 = src32;
	do {
		c = *(r32++);
		::_MCFCRT_EncodeUtf8(&w8, c);
	} while(c);
	std::printf("utf32 -> utf8 , len = %td, cmp = %d\n", w8 - dst8, std::memcmp(src8, dst8, sizeof(src8)));

	std::memset(dst16, -1, sizeof(dst16));
	w16 = dst16;
	r32 = src32;
	do {
		c = *(r32++);
		::_MCFCRT_EncodeUtf16(&w16, c);
	} while(c);
	std::printf("utf32 -> utf16, len = %td, cmp = %d\n", w16 - dst16, std::memcmp(src16, dst16, sizeof(src16)));

	std::memset(dst8, -1, sizeof(dst8));
	w8 = dst8;
	r16 = src16;
	do {
		c = ::_MCFCRT_EncodeUtf8FromUtf16(&w8, &r16);
	} while(c);
	std::printf("utf16 -> utf8 , len = %td, cmp = %d\n", w8 - dst8, std::memcmp(src8, dst8, sizeof(src8)));

	std::memset(dst16, -1, sizeof(dst16));
	w16 = dst16;
	r8 = src8;
	do {
		c = ::_MCFCRT_EncodeUtf16FromUtf8(&w16, &r8);
	} while(c);
	std::printf("utf8  -> utf16, len = %td, cmp = %d\n", w16 - dst16, std::memcmp(src16, dst16, sizeof(src16)));

	return 0;
}
