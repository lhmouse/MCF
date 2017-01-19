#include <MCF/StdMCF.hpp>
#include <cmath>

volatile auto pz = +0.0;
volatile auto nz = -0.0;

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	__builtin_printf("%f\n", std::cbrt(pz));
	__builtin_printf("%f\n", std::cbrt(nz));
	return 0;
}
