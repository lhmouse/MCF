#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/UniquePtr.hpp>
using namespace MCF;

template class UniquePtr<const volatile int>;
template class UniquePtr<const int>;
template class UniquePtr<volatile int>;
template class UniquePtr<int>;

extern "C" unsigned int MCFMain() noexcept {
	UniquePtr<int> p;
	p = MakeUnique<int>();

	UniquePtr<int[]> p1(new int[2]);
	p1[0] = 123;
	UniquePtr<const int[]> p2(std::move(p1));
	std::printf("%d %d\n", p2[0], p1 == p2);
	return 0;
}
