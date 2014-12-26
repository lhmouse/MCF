#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/PolymorphicIntrusivePtr.hpp>
#include <MCF/Core/Exception.hpp>
using namespace MCF;

struct foo : PolymorphicIntrusiveBase<foo> {
	int i;

	explicit foo(int k){
		i = k;
	}

	foo(const foo &) = delete;
	foo(foo &&) = default;
};

extern "C" unsigned int MCFMain() noexcept {
	try {
		PolymorphicIntrusivePtr<foo> p = MakePolymorphicIntrusive<foo>(foo{12345});
		auto p2 = DynamicClone(p);
		std::printf("p2 = %p, p2->i = %d\n", p2.Get(), p2->i);
	} catch(Exception &e){
		std::printf("exception: %s\n", e.what());
	}
	return 0;
}
