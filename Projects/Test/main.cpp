#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/PolyIntrusivePtr.hpp>
#include <iostream>
#include <string>

using namespace MCF;

struct foo : PolyIntrusiveBase<foo> {
};
struct bar : PolyIntrusiveBase<bar>, foo {
};

extern "C" unsigned int MCFMain() noexcept {
	PolyIntrusivePtrUnknown p = MakePolyIntrusive<bar>();

	std::printf("typeid of p is %s\n", typeid(p).name());
	std::printf("typeid of *p is %s\n", typeid(*p).name());

	return 0;
}
