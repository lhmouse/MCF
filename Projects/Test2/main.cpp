#include <MCF/StdMCF.hpp>
#include <MCF/Random/IsaacRng.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	IsaacRng rng(100);
	for(int i = 0; i < 10; ++i){
		std::printf("%08X\n", rng.Get());
	}
	return 0;
}
