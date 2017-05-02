#include <stdio.h>

volatile auto f = __builtin_fmin;

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	__mingw_printf("%g\n", f(1.0, -2.0));
	return 0;
}
