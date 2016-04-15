#include <MCF/StdMCF.hpp>
#include <MCF/Core/Clocks.hpp>
#include <cstdio>

extern "C" unsigned _MCFCRT_Main(){
	std::printf("val = %lu\n", (unsigned long)_MCFCRT_ReadTimestampCounter32());
	return 0;
}
