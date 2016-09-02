#include <MCF/StdMCF.hpp>
#include <MCF/Core/TupleManipulators.hpp>
#include <cstdio>

void foo(int &){
	std::puts("foo(int &)");
}
void foo(int &&){
	std::puts("foo(int &&)");
}

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	int n1 = 1, n2 = 2, n3 = 3;

	std::puts(">> testing lvalues!");
	MCF::AbsorbTuple([](auto &&a){ foo(static_cast<decltype(a) &&>(a)); }, std::tie(n1, n2, n3));

	std::puts(">> testing xvalues!");
	MCF::AbsorbTuple([](auto &&a){ foo(static_cast<decltype(a) &&>(a)); }, std::forward_as_tuple(std::move(n1), std::move(n2), std::move(n3)));

	std::puts(">> testing prvalues!");
	MCF::AbsorbTuple([](auto &&a){ foo(static_cast<decltype(a) &&>(a)); }, std::make_tuple(1, 2, 3));

	return 0;
}
