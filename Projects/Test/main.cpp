#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/PolymorphicSharedPtr.hpp>
#include <MCF/Core/Exception.hpp>
using namespace MCF;

struct foo {
	int i;

	foo(const foo &) = delete;
	foo(foo &&) = default;
};

extern "C" unsigned int MCFMain() noexcept {
	try {
		PolymorphicSharedPtr<void> p = MakePolymorphicShared<foo>(foo{12345});
		auto p2 = DynamicClone(p);
		auto pi = DynamicPointerCast<foo>(p2);
		std::printf("pi = %p, pi->i = %d\n", pi.Get(), pi->i);
	} catch(Exception &e){
		std::printf("exception: %s\n", e.what());
	}
	return 0;
}
