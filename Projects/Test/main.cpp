#include <MCF/StdMCF.hpp>
#include <MCFCRT/ext/itoa.h>
#include <MCFCRT/ext/atoi.h>
#include <cstdio>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	char str[256] = "-2147483648.123456";
	char *ep;
	::_MCFCRT_atoi_result r;
	std::intptr_t v;
	ep = ::_MCFCRT_atoi_d(&r, &v, str);
	std::printf("r = %d, v = %td, chars_eaten = %td\n", r, v, ep - str);
	ep = ::_MCFCRT_atoi0d(&r, &v, str, 4);
	std::printf("r = %d, v = %td, chars_eaten = %td\n", r, v, ep - str);
/*	ep = ::_MCFCRT_itoa_d(str, INT_MIN);
	std::memcpy(ep, "_meow", 6);
	std::printf("str = %s\n", str);
*/	return 0;
}
