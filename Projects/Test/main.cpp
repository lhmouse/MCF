#include <MCF/StdMCF.hpp>
#include <MCF/Function/Function.hpp>
#include <iostream>

extern "C" unsigned MCFCRT_Main(){
	auto l = [](auto n) -> bool { return std::cout <<"n = " <<n <<std::endl; };

	auto f1 = MCF::Function<const volatile void (int)>(l);
	f1(123);

	auto f2 = MCF::Function<int (int)>(l);
	auto b = f2(456);
	std::cout <<"b = " <<std::boolalpha <<b <<std::endl;

	return 0;
}
