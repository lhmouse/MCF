#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/SharedPtr.hpp>
#include <string>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	WeakPtr<std::string> p;
	{
		SharedPtr<std::string> p1;
		p1 = MakeShared<std::string>("hello world!");
		{
			SharedPtr<const std::string> p2(p1);
			p = p1;

			std::printf("%u %u: %s\n", p2.GetWeakCount(), p2.GetSharedCount(), p2->c_str());

			std::printf("weak: %u %u: %p\n", p.GetWeakCount(), p.GetSharedCount(), [&]{ return p.Lock().Get(); }());
		}
			std::printf("weak: %u %u: %p\n", p.GetWeakCount(), p.GetSharedCount(), [&]{ return p.Lock().Get(); }());
	}
			std::printf("weak: %u %u: %p\n", p.GetWeakCount(), p.GetSharedCount(), [&]{ return p.Lock().Get(); }());
	return 0;
}
