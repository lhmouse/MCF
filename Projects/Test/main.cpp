#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/PolymorphicSharedPtr.hpp>
#include <string>
using namespace MCF;

template class SharedPtr<std::string>;
template class WeakPtr<std::string>;

extern "C" unsigned int MCFMain() noexcept {
	PolymorphicWeakPtr<std::string> p;
	{
		PolymorphicSharedPtr<std::string> p1;
		p1 = MakePolymorphicShared<std::string>("hello world!");
		{
			PolymorphicSharedPtr<const std::string> p2(p1);
			p = p1;

			std::printf("%u %u: %s\n", p2.GetWeakCount(), p2.GetSharedCount(), p2->c_str());

			std::printf("weak: %u %u: %p\n", p.GetWeakCount(), p.GetSharedCount(), [&]{ return p.Lock().Get(); }());
		}
			std::printf("weak: %u %u: %p\n", p.GetWeakCount(), p.GetSharedCount(), [&]{ return p.Lock().Get(); }());
	}
			std::printf("weak: %u %u: %p\n", p.GetWeakCount(), p.GetSharedCount(), [&]{ return p.Lock().Get(); }());
	return 0;
}
