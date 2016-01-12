#include <MCF/StdMCF.hpp>
#include <MCF/Utilities/TupleManipulators.hpp>
#include <iostream>

extern "C" unsigned MCFMain(){
	auto l = [](auto a){
		std::cout <<a <<" " <<std::endl;
	};
	MCF::AbsorbTuple(l, std::forward_as_tuple(1, 2.3, "4", '5'));

	return 0;
}
