#include <MCF/StdMCF.hpp>
#include <MCF/Random/IsaacRng.hpp>
#include <MCF/Core/Time.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	IsaacRng rng(0);

	const auto begin = GetHiResMonoClock();
	for(unsigned i = 0; i < 100'000'000; ++i){
		rng.Get();
	}
	const auto end = GetHiResMonoClock();

	std::printf("time elasped: %f ms\n", end - begin);
	return 0;
}
