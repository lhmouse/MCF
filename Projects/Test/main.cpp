#include <MCF/StdMCF.hpp>
#include <MCF/Core/Clocks.hpp>
#include <cstdio>

extern "C" unsigned MCFCRT_Main(){
	using Ull = unsigned long long;

	Ull local = MCF::GetLocalClock();
	Ull utc   = MCF::GetUtcClock();
	std::printf("local = %llu\n", local);
	std::printf("utc   = %llu\n", utc);

	return 0;
}
