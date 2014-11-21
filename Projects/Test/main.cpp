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

template class IntrusivePtr<foo>;

extern "C" unsigned int MCFMain() noexcept {
	std::puts("------ 1");
	IntrusivePtr<foo> p(new foo);
	std::puts("------ 2");
	IntrusivePtr<const foo> p2(p);
	std::puts("------ 3");
	auto raw = p.Release();
	std::puts("------ 4");
	auto forked = p2.Fork();
	std::printf("%p\n", forked.Get());
	p2.Reset();
	std::puts("------ 5");
	raw->DropRef();
	std::puts("------ 6");
	forked.Reset();
	std::puts("------ 7");
	return 0;
}
