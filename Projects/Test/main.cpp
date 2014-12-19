#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/UniquePtr.hpp>
using namespace MCF;

template class UniquePtr<const volatile int>;
template class UniquePtr<volatile int>;
template class UniquePtr<const int>;
template class UniquePtr<int>;

template class UniquePtr<const volatile int []>;
template class UniquePtr<volatile int []>;
template class UniquePtr<const int []>;
template class UniquePtr<int []>;

extern "C" unsigned int MCFMain() noexcept {
	UniquePtr<int[]> p1;
//	p1 = MakeUnique<int>(123);
	p1.Reset(new int[5]{ 1, 2, 3, 4, 5 });
	UniquePtr<const int[]> p2(std::move(p1));
//	std::printf("%d\n", *p2);
	std::printf("%d\n", p2[3]);
	return 0;
}
