#include <cstdio>
#include <cmath>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	volatile auto f = 123.456;
	std::printf("u = %f\n", __builtin_ceil(f));
	return 0;
}
