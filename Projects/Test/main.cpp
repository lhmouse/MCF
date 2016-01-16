#include <MCF/StdMCF.hpp>
#include <MCF/Function/Function.hpp>
#include <iostream>

extern "C" unsigned MCFCRT_Main(){
	auto fn = MCF::Function<int (int, int)>([](int a, int){ return a + 1; });
	std::cout <<"fn(2) = " <<fn(2, 3) <<std::endl;
	return 0;
}
