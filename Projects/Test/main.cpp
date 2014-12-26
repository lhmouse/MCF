#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/PolymorphicSharedPtr.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	PolymorphicSharedPtr<const void> p = MakePolymorphicShared<int>(123456);

	auto pd = DynamicPointerCast<const double>(p);
	auto pi = DynamicPointerCast<const int>(p);

	std::printf("pd = %p\n", (void *)pd.Get());
	std::printf("pi = %p, *pi = %d\n", (void *)pi.Get(), *pi);

	PolymorphicWeakPtr<const void> wp(std::move(p));
	wp.Lock();

	return 0;
}
