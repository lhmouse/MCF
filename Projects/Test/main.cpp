#include <stdio.h>
#include <math.h>

volatile auto ptr = ::tanl;

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	__mingw_printf("tan(2) = %Lg\n", ptr(2));
	return 0;
}
