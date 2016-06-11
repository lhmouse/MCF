#include <MCF/StdMCF.hpp>
#include <MCFCRT/ext/div64.h>
#include <cstdio>

constexpr std::uint64_t a = 987654321987;
constexpr std::uint32_t b = 65432178;

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	std::uint32_t quo, rem;
	quo = _MCFCRT_udivrem64(&rem, a, b);
	std::printf("quo = %llu, rem = %llu\n", (unsigned long long)quo, (unsigned long long)rem);
	return 0;
}
