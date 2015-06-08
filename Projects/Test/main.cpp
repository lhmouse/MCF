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
	PolymorphicIntrusivePtr<volatile foo> p3 = p1;
	PolymorphicIntrusivePtr<const volatile foo> p4 = p1;

	std::cout <<"ref = " <<p1.GetSharedCount() <<std::endl;

	return 0;
}
