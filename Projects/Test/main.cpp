#include <MCF/StdMCF.hpp>
#include <MCF/Random/IsaacGenerator.hpp>

using namespace MCF;

extern "C" unsigned MCFCRT_Main(){
	IsaacGenerator rng(100);
	for(unsigned i = 0; i < 10; ++i){
		std::printf("val = %lu\n", (unsigned long)rng());
	}
	return 0;
}
