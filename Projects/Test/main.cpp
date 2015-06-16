#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Function/Bind.hpp>
#include <iostream>

using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	AnsiString str;
	auto fn = Bind([](auto &str, auto s){ str += s; }, Ref(str), _1);
	fn("hello ");
	fn("world!");
	std::cout <<str.GetStr() <<std::endl;
	return 0;
}
