#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/IntrusivePtr.hpp>
#include <iostream>
#include <string>

using namespace MCF;

struct foo : IntrusiveBase<foo> {
	foo(int, const char *){
	}
};

extern "C" unsigned int MCFMain() noexcept {
	IntrusivePtr<foo> p1 = MakeIntrusive<foo>(1, "a");
	IntrusivePtr<const foo> p2 = p1;
	IntrusivePtr<volatile foo> p3 = p1;
	IntrusivePtr<const volatile foo> p4 = p1;

	std::cout <<"ref = " <<p1.GetSharedCount() <<std::endl;

	return 0;
}
