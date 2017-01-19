#include <MCF/StdMCF.hpp>
#include <cmath>

volatile double d = 27;

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	__builtin_printf("%f\n", std::cbrt(+0.0));
	__builtin_printf("%f\n", std::cbrt(-0.0));
	__builtin_printf("%f\n", std::cbrt(+INFINITY));
	__builtin_printf("%f\n", std::cbrt(-INFINITY));
	__builtin_printf("%f\n", std::cbrt(+d));
	__builtin_printf("%f\n", std::cbrt(-d));
	return 0;
}
