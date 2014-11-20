#include <MCF/StdMCF.hpp>
#include <MCF/Core/IntrusivePtr.hpp>
using namespace MCF;

struct foo : IntrusiveBase<foo> {
	foo(){
		std::puts("foo::foo()");
	}
	~foo(){
		std::puts("foo::~foo()");
	}
};

extern "C" unsigned int MCFMain() noexcept {
	std::puts("------ 1");
	IntrusivePtr<foo> p(new foo);
	std::puts("------ 2");
	IntrusivePtr<const foo> p2(p);
	std::puts("------ 3");
	p.Reset();
	std::puts("------ 4");
	p2.Reset();
	std::puts("------ 5");
	return 0;
}
