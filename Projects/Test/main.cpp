#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/PolymorphicIntrusivePtr.hpp>
using namespace MCF;

struct foo : PolymorphicIntrusiveBase {
	int i;

	foo(int k) : i(k) {
	}
};
struct bar : foo {
	double d;

	bar() : foo(123), d(45.6) {
	}
	~bar(){
		std::puts("~bar()");
	}
};
struct meow : foo {
	~meow(){
		std::puts("~meow()");
	}
};

extern "C" unsigned int MCFMain() noexcept {
	PolymorphicIntrusivePtr<foo> p;
	{
		auto p2 = MakePolymorphicIntrusive<bar>();
		p2->AddRef();
		p.Reset(p2.Get());
	}
	auto p3 = DynamicPointerCast<meow>(std::move(p));
	std::puts("---------------");
	std::printf("p3 = %p\n", (void *)p3.Get());
//	std::printf("%d\n", p->i);
	return 0;
}
