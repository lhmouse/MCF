#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/PolymorphicIntrusivePtr.hpp>
#include <iostream>
#include <string>

using namespace MCF;

struct foo : PolymorphicIntrusiveBase<foo> {
	foo(int, const char *){
	}
};

extern "C" unsigned int MCFMain() noexcept {
	PolymorphicIntrusivePtr<foo> p1 = MakePolymorphicIntrusive<foo>(1, "a");
	PolymorphicIntrusivePtr<const foo> p2 = p1;
	PolymorphicIntrusivePtr<volatile foo> p3 = std::move(p1);
	PolymorphicIntrusivePtr<const volatile foo> p4 = p2;
	PolymorphicIntrusivePtr<foo> p5 = DynamicClone(p2);

	std::printf("ref1 = %zu\n", p1.GetSharedCount());
	std::printf("ref2 = %zu\n", p2.GetSharedCount());
	std::printf("ref3 = %zu\n", p3.GetSharedCount());
	std::printf("ref4 = %zu\n", p4.GetSharedCount());
	std::printf("ref5 = %zu\n", p5.GetSharedCount());

	return 0;
}
