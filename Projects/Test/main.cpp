#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/PolymorphicIntrusivePtr.hpp>
#include <MCF/Core/Exception.hpp>
using namespace MCF;

struct foo : PolymorphicIntrusiveBase<foo> {
	int i;

	explicit foo(int k){
		i = k;
	}

	foo(const foo &) = default;
	foo(foo &&) = default;
};

struct bar : PolymorphicIntrusiveBase<foo> {
};

extern "C" unsigned int MCFMain() noexcept {
	try {
		PolymorphicIntrusivePtr<const foo> p = MakePolymorphicIntrusive<const foo>(foo{12345});
		auto p2 = DynamicClone(p);
		p2->i = 45678;
		std::printf("p = %p, p->i = %d\n", p.Get(), p->i);
		std::printf("p2 = %p, p2->i = %d\n", p2.Get(), p2->i);
	} catch(Exception &e){
		std::printf("exception: %s\n", e.what());
	}
	return 0;
}
