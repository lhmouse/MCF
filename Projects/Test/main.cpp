#include <MCF/StdMCF.hpp>
#include <MCF/Thread/CriticalSection.hpp>
using namespace MCF;

auto p = CriticalSection::Create();

extern "C" unsigned int MCFMain() noexcept {
	auto l1 = p->TryLock();
	auto l2 = p->TryLock();
	std::printf("l1: %d, l2: %d\n", !!l1, !!l2);
	return 0;
}
