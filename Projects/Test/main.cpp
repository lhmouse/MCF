#include <MCF/StdMCF.hpp>
#include <cstdio>

using namespace MCF;

volatile int fl = 1;

extern "C" unsigned int MCFMain() noexcept {
	try {
		if(fl){
			std::puts("about to throw...");
			throw std::out_of_range("test out_of_range");
		}
	} catch(std::exception &e){
		std::printf("exception caught: what = %s\n", e.what());
	}
	return 0;
}

extern "C" {

__attribute__((__section__(".eh_frame$@@@"), __used__))
extern const uintptr_t __MCF_CRT_EhFrameBegin[1] = { };

}
