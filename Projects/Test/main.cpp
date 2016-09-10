#include <MCF/StdMCF.hpp>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	volatile long double d = 1;
	for(unsigned i = 0; i < 10; ++i){
		d *= 1e100;
	}
	__builtin_printf("%f\n", (double)d);
	return 0;
}
