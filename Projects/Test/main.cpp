#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/SharedPtr.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	SharedPtr<int> sp1(new int), sp2(sp1);
	WeakPtr<int> wp(sp1);

	std::printf("%p\n", wp.Lock().Get());

	sp1.Reset();
	std::printf("%p\n", wp.Lock().Get());

	sp2.Reset();
	std::printf("%p\n", wp.Lock().Get());

	return 0;
}
