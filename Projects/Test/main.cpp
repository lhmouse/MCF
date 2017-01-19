#include <MCF/StdMCF.hpp>
#include <cstdio>
#include <cmath>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	volatile double val = __builtin_nan("0x12345");
	std::printf("%g\n", std::fmax(1, val));
	return 0;
}
