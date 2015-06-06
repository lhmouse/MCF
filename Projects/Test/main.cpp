#include <MCF/StdMCF.hpp>
#include <MCF/Function/Bind.hpp>
#include <iostream>
#include <functional>

using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	auto fn = Bind(std::multiplies<int>(), 3, _1);

	std::cout <<fn(2) <<std::endl;
	std::cout <<fn(3) <<std::endl;

	std::string str = "hello";
	Bind([](auto p){ *p += " world!"; }, &str)();
	std::cout <<str <<std::endl;

	return 0;
}
