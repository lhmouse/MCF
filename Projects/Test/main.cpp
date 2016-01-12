#include <MCF/StdMCF.hpp>
#include <MCF/Function/Bind.hpp>

extern "C" unsigned MCFMain(){
	auto f1 = MCF::LazyBind([](auto a, auto b){ return a + b; }, MCF::_1, 2);
	auto f2 = MCF::LazyBind([](auto a, auto b){ return a * b; }, f1, MCF::_1);
	std::printf("f2() = %d\n", f2(3));

	return 0;
}
