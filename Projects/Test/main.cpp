#include <MCF/StdMCF.hpp>
#include <MCF/Random/IsaacGenerator.hpp>

using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	IsaacGenerator gen(0);
	for(unsigned i = 0; i < 10; ++i){
		std::printf("%08lX\n", (unsigned long)gen());
	}
	return 0;
}
