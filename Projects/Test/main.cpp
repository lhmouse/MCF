#include <MCF/StdMCF.hpp>
#include <MCF/Thread/Mutex.hpp>
#include <iostream>
#include <string>

using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	Mutex m;

	Mutex::UniqueLock l1(m);
	l1.Unlock();

	Mutex::UniqueLock l2(m);
	l2.Unlock();

	return 0;
}
