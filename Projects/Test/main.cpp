#include <MCF/StdMCF.hpp>
#include <MCF/Thread/Semaphore.hpp>
#include <cstdio>

using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	Semaphore sem(2);
	unsigned cnt = sem.BatchWait(5000, 3);
	std::printf("cnt = %u\n", cnt);
	return 0;
}
