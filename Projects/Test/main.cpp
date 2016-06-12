#include <MCF/StdMCF.hpp>
#include <cstdio>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	long long a = 1234567890123456789;
	long double b = a;
	return 0;
}
