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
	PolymorphicIntrusivePtr<const volatile foo> p4 = std::move(p2);

	std::cout <<"ref1 = " <<p1.GetSharedCount() <<std::endl;
	std::cout <<"ref2 = " <<p2.GetSharedCount() <<std::endl;
	std::cout <<"ref3 = " <<p3.GetSharedCount() <<std::endl;
	std::cout <<"ref4 = " <<p4.GetSharedCount() <<std::endl;

	return 0;
}
