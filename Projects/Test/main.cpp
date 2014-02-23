#include <MCF/Core/Random.hpp>
#include <cstdio>
#include <cstdlib>

unsigned int MCFMain(){
	MCF::Random rng;
	for(std::size_t i = 0; i < 10; ++i){
		std::printf("%08X\n", rng.Get<unsigned int>());
	}
	return 0;
}
