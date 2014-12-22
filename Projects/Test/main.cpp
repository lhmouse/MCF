#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/PolymorphicSharedPtr.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	PolymorphicSharedPtr<void> p = MakePolymorphicShared<int>(123456);

	auto p1 = DynamicPointerCast<int>(p);
	std::printf("p1 = %p, *p1 = %d\n", p1.Get(), *p1);

	auto p2 = DynamicPointerCast<double>(p);
	std::printf("p2 = %p\n", p2.Get());

	return 0;
}
