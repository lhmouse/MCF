#include <MCF/StdMCF.hpp>
#include <MCF/Thread/Semaphore.hpp>
#include <cstdio>

using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	try {
		throw std::bad_alloc();
	} catch(std::exception &e){
		std::printf("exception caught: what = %s\n", e.what());
	}
	return 0;
}
