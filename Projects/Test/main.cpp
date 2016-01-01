#include <MCF/StdMCF.hpp>
#include <MCF/Core/Optional.hpp>
#include <iostream>
#include <string>

using namespace MCF;

template class Optional<std::string>;
template class Optional<std::exception_ptr>;

extern "C" unsigned MCFMain(){
	Optional<std::string> opt(std::forward_as_tuple("hello world!", 5u));
	Optional<std::string> opt2 = opt;
	std::cout <<opt2.Get() <<std::endl;
	return 0;
}
