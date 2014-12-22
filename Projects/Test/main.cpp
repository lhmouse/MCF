#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/PolymorphicSharedPtr.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	WeakPtr<int> wp;
	SharedPtr<int> sp;
	sp.Reset(new int(123));
	wp = sp;
	std::printf("sp = %p, wp = %p\n", sp.Get(), wp.Lock().Get());
	sp.Reset();
	std::printf("sp = %p, wp = %p\n", sp.Get(), wp.Lock().Get());
	return 0;
}
