#include <cstdio>
#include <cmath>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	volatile auto p = __builtin_fabs;
	volatile auto f = 123.456;
	std::printf("u = %f\n", p(f));
	return 0;
}
