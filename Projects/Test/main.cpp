#include <MCF/StdMCF.hpp>
#include <cstdio>

using namespace MCF;

volatile int fl = 1;

extern "C" unsigned int MCFMain() noexcept {
	try {
		if(fl){
			throw std::bad_alloc();
		}
	} catch(std::exception &e){
		std::printf("exception caught: what = %s\n", e.what());
	}
	return 0;
}
