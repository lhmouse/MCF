#include <MCF/StdMCF.hpp>
#include <cstdio>
#include <cmath>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	volatile double val = __builtin_nan("0x12345");
	std::printf("%g\n", std::hypot(val, HUGE_VAL));
	return 0;
}
