#include <MCF/StdMCF.hpp>
#include <MCFCRT/env/bail.h>
#include <cstdio>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	try {
		throw 12345;
	} catch(int e){
		std::printf("exception caught: e = %d\n", e);
	}
	return 0;
}
