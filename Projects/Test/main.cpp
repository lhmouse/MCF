#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/IntrusivePtr.hpp>
#include <cstdio>

using namespace MCF;

struct foo;

IntrusivePtr<foo> gp;

struct foo : IntrusiveBase<foo> {
	foo(){
		gp = this->Share();

		std::puts("foo::foo()");
	}
	~foo(){
		std::puts("foo::~foo()");
	}
};

extern "C" unsigned int MCFMain() noexcept {
	IntrusivePtr<foo> p(new foo);
	p.Reset();

	std::printf("about to reset gp..., gp = %p\n", dynamic_cast<void *>(gp.Get()));
 	gp.Reset();
	std::puts("done resetting gp...");

	return 0;
}
