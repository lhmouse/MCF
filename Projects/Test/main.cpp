#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/PolyIntrusivePtr.hpp>
#include <iostream>
#include <string>

using namespace MCF;

struct foo : PolyIntrusiveBase<foo> {
	int i = 12345;
};
struct bar : foo {
};

extern "C" unsigned int MCFMain() noexcept {
	PolyIntrusivePtrUnknown p1 = MakePolyIntrusive<foo>();
	PolyIntrusivePtr<foo> p2 = DynamicPointerCast<foo>(p1);
	PolyIntrusivePtr<foo> p3 = StaticPointerCast<foo>(p2);
	PolyIntrusivePtr<bar> p4 = DynamicPointerCast<bar>(p3);

	std::printf("p1 = %p\n", p1.Get());
	std::printf("p2 = %p, i = %d\n", p2.Get(), p2->i);
	std::printf("p3 = %p, i = %d\n", p3.Get(), p3->i);
	std::printf("p4 = %p\n", p4.Get());

	return 0;
}
