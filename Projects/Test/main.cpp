#include <MCF/StdMCF.hpp>
#include <MCF/Core/Clocks.hpp>
#include <cstdio>

extern "C" unsigned MCFCRT_Main(){
	auto t1 = MCF::GetHiResMonoClock();
	::Sleep(1000);
	auto t2 = MCF::GetHiResMonoClock();
	std::printf("t2 - t1 = %f\n", t2 - t1);

	return 0;
}
