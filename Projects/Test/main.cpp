#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/PolymorphicSharedPtr.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	auto p1 = MakePolymorphicShared<int>();
	auto p2 = MakePolymorphicShared<long long>();
	auto p3 = MakePolymorphicShared<double>();
	return 0;
}
