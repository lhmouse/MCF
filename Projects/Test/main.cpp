#include <cstdio>
#include <cmath>
#include <cfloat>

volatile double a = __builtin_nan("0xaaaa"), b = __builtin_nan("0xbbbb");

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	std::printf("r = %f\n", std::hypot(a, b));
	return 0;
}
