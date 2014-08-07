#include <MCF/StdMCF.hpp>
#include <MCF/Random/IsaacRng.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	IsaacRng rng(0);
	for(std::size_t i = 0; i < 768; i += 8){
		for(std::size_t j = 0; j < 8; ++j){
			std::printf("%08x", rng.Get());
		}
		std::putchar('\n');
	}
	return 0;
}
