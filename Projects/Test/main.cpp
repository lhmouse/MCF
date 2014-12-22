#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/PolymorphicSharedPtr.hpp>
using namespace MCF;

template class SharedPtr<void, DefaultDeleter<Impl::PolymorphicSharedPtrContainerBase>>;
template class WeakPtr<void, DefaultDeleter<Impl::PolymorphicSharedPtrContainerBase>>;

template class SharedPtr<int, DefaultDeleter<Impl::PolymorphicSharedPtrContainerBase>>;
template class WeakPtr<int, DefaultDeleter<Impl::PolymorphicSharedPtrContainerBase>>;

template class SharedPtr<int [], DefaultDeleter<Impl::PolymorphicSharedPtrContainerBase>>;
template class WeakPtr<int [], DefaultDeleter<Impl::PolymorphicSharedPtrContainerBase>>;

extern "C" unsigned int MCFMain() noexcept {
	PolymorphicWeakPtr<void> wp;
	PolymorphicSharedPtr<void> p = MakePolymorphicShared<int>(123456);
	wp = p;

	auto p1 = DynamicPointerCast<int>(p);
	std::printf("p1 = %p, *p1 = %d\n", p1.Get(), *p1);

	auto p2 = DynamicPointerCast<double>(p);
	std::printf("p2 = %p\n", p2.Get());

	return 0;
}
