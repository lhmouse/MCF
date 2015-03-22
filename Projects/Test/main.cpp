#include <MCF/StdMCF.hpp>
#include <MCF/Random/FastGenerator.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	FastGenerator rng;
	for(unsigned i = 0; i < 20; ++i){
		std::printf("%08X\n", (unsigned)rng.Get());
	}
	return 0;
}
