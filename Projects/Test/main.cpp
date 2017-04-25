#include <stdio.h>

volatile long double f = __builtin_nansl("");

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	__mingw_printf("f = %Lg\n", f + 1);
	return 0;
}
