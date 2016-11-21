#include <MCF/StdMCF.hpp>
#include <math.h>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	volatile auto pf = ::hypotl;
	::printf("%Lf\n", pf(LDBL_MIN * 2, LDBL_MIN * 2));
	return 0;
}
