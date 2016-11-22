#include <MCF/StdMCF.hpp>
#include <math.h>
#include <iostream>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	volatile auto pf = ::hypotl;
	std::cout <<pf(LDBL_MAX / 2, LDBL_MAX / 2) <<std::endl
	          <<pf(LDBL_MAX / 2, LDBL_MIN * 2) <<std::endl
	          <<pf(LDBL_MIN * 2, LDBL_MAX / 2) <<std::endl
	          <<pf(LDBL_MIN * 2, LDBL_MIN * 2) <<std::endl;
	return 0;
}
