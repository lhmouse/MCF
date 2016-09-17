#include <MCF/StdMCF.hpp>
#include <MCF/Core/Function.hpp>
#include <cstdio>

template class MCF::Function<int (int)>;

struct foo {
	foo() = default;
	foo(const foo &) = delete;
	foo(foo &&) = default;

	int operator()(int a) const {
		return a * a;
	}
};

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	auto f1 = MCF::Function<int (int)>(foo());
	auto f2 = f1;
	f2.Clone();
	return 0;
}
