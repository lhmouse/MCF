#include <MCF/StdMCF.hpp>
#include <MCF/Function/Function.hpp>

using namespace MCF;

struct foo {
	foo(){
		std::puts(__PRETTY_FUNCTION__);
	}
	foo(const foo &){
		std::puts(__PRETTY_FUNCTION__);
	}
	foo(foo &&) noexcept {
		std::puts(__PRETTY_FUNCTION__);
	}
	~foo(){
		std::puts(__PRETTY_FUNCTION__);
	}
};

struct bar {
	void operator()(foo) const {
		std::puts(__PRETTY_FUNCTION__);
	}
};

extern "C" unsigned MCFMain(){
	std::puts("--- test 1 ---");
	Function<void (foo)> f;
	f.Reset(bar());
	f(foo());

	std::puts("--- test 2 ---");
	bar b;
	b(foo());

	return 0;
}
