#include <MCF/StdMCF.hpp>
#include <MCF/Function/Function.hpp>
#include <MCF/Function/Bind.hpp>
#include <iostream>

using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	std::string s;

	auto fn = Bind(&std::string::push_back, &s, _1);

	fn('a');
	fn('b');
	std::cout <<s;

	return 0;
}
