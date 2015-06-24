#include <MCF/StdMCF.hpp>
#include <cstdio>

using namespace MCF;

extern "C" {

__extension__ __attribute__((__section__(".eh_frame$@@@"), __used__))
extern const intptr_t __eh_frame_begin[0] = { };

}

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
