#include <MCF/StdMCF.hpp>
#include <MCF/Function/Function.hpp>

using namespace MCF;

struct foo {
	foo(){
		std::puts("foo::foo()");
	}
	foo(const foo &){
		std::puts("foo::foo(const foo &)");
	}
	foo(foo &&) noexcept {
		std::puts("foo::foo(foo &&)");
	}
	~foo(){
		std::puts("foo::~foo()");
	}

	void operator()() const {
		std::puts("foo::operator()()");
	}
};

extern "C" unsigned MCFMain(){
	auto f1 = Function<void ()>(foo());
	auto f2 = f1;

	std::puts("-- Ready to fork!");
	f2.Fork();

	return 0;
}
