#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/PolymorphicIntrusivePtr.hpp>
#include <iostream>
#include <string>

using namespace MCF;

struct foo : PolymorphicIntrusiveBase<foo> {
};
struct bar : PolymorphicIntrusiveBase<bar>, foo {
};

extern "C" unsigned int MCFMain() noexcept {
	PolymorphicIntrusivePtrUnknown p = MakePolymorphicIntrusive<bar>();

	std::printf("typeid of p is %s\n", typeid(p).name());
	std::printf("typeid of *p is %s\n", typeid(*p).name());

	return 0;
}
