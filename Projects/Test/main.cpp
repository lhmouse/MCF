#include <MCF/Function/Function.hpp>
#include <iostream>

using namespace MCF;

struct foo {
	foo() = default;
	foo(const foo &) = default; // delete;
	foo(foo &&) noexcept = default;

	int operator()(int a) const {
		return a * 2;
	}
};

extern "C" unsigned MCFMain(){
	auto f1 = MCF::Function<int (int)>(foo());
	auto f2 = f1;
	std::printf("f2 = %d\n", f2(123));
	f2.Fork();
	std::printf("f2 = %d\n", f2(123));
	return 0;
}
