#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/PolymorphicIntrusivePtr.hpp>
#include <iostream>
#include <string>

using namespace MCF;

struct foo : PolymorphicIntrusiveBase<foo> {
};
struct bar : foo {
};

extern "C" unsigned int MCFMain() noexcept {
	PolymorphicIntrusivePtr<foo> p1 = MakePolymorphicIntrusive<bar>();
	auto p2 = DynamicClone(p1);

	std::printf("typeid of p2 is %s\n", typeid(p2).name());
	std::printf("typeid of *p2 is %s\n", typeid(*p2).name());

	return 0;
}
