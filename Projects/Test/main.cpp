#include <MCF/StdMCF.hpp>
#include <MCF/Function/Function.hpp>
#include <MCF/Function/Bind.hpp>
#include <iostream>

using namespace MCF;

extern "C" unsigned MCFMain(){
	Function<void(int, bool)> f;
	f = [](auto x, auto y){
		std::cout <<std::boolalpha <<"x = " <<x <<", y = " <<y <<std::endl;
	};
	f(123, true);

	return 0;
}
