#include <MCF/StdMCF.hpp>
#include <cstdio>

extern "C" char *_MCFCRT_atoi_d(std::intptr_t *pnValue, const char *pchBuffer) noexcept;
extern "C" char *_MCFCRT_atoi0d(std::intptr_t *pnValue, const char *pchBuffer, unsigned uMaxDigits) noexcept;

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	char str[32] = "2147483647";
	char *ep;
	std::intptr_t value;
	ep = _MCFCRT_atoi_d(&value, str);
	std::printf("value = %td, chars_eaten = %td\n", value, ep - str);
	ep = _MCFCRT_atoi0d(&value, str, 4);
	std::printf("value = %td, chars_eaten = %td\n", value, ep - str);
	return 0;
}
