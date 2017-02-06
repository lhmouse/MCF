#include <MCF/Function/Function.hpp>
#include <cstdio>

using namespace MCF;

struct foo {
	foo() noexcept {
		std::puts(__PRETTY_FUNCTION__);
	}
	foo(const foo &) noexcept {
		std::puts(__PRETTY_FUNCTION__);
	}
	foo(foo &&) noexcept {
		std::puts(__PRETTY_FUNCTION__);
	}
	~foo(){
		std::puts(__PRETTY_FUNCTION__);
	}
};

int i = 3;
auto cl = [&](int j, foo){ return i * j; };
auto fn = MakeFunction<int (int, foo)>(cl);

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	i += 2;
	std::printf("n = %d\n", (*fn)(7, foo()));
	return 0;
}
