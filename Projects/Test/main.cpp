#include <MCF/StdMCF.hpp>
#include <MCF/Function/Function.hpp>
#include <iostream>

using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	Function<int (int, int)> fn([](auto i, auto j){ return i + j; });

	std::cout <<fn(1, 2) <<std::endl;
	std::cout <<fn(3, 4) <<std::endl;

	return 0;
}
