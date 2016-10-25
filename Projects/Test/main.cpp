#include <cstdio>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	float f1 = 1.234, f2 = 1;
	volatile auto pff = ::__builtin_copysignf;
	std::printf("copysignf(f1, f2) = %f\n", pff(f1, f2));
	double d1 = 5.678, d2 = -2;
	volatile auto pfd = ::__builtin_copysign;
	std::printf("copysign (d1, d2) = %f\n", pfd(d1, d2));
	return 0;
}
