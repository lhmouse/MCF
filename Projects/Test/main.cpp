#include <MCF/StdMCF.hpp>
#include <MCF/Random/FastGenerator.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	for(unsigned i = 0; i < 100; ++i){
		std::printf("%08lX\n", (unsigned long)FastGenerator::GlobalGet());
	}
	return 0;
}
