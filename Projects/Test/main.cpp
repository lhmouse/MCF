#include <MCF/Core/Optional.hpp>
#include <string>
#include <cstdio>

using namespace MCF;

template class Optional<std::string>;

__attribute__((__noinline__))
Optional<std::string> foo(){
	Optional<std::string> opt("hello world!");
	return opt;
}

extern "C" unsigned MCFMain(){
	auto opt = foo();
	std::printf("set = %d, str = %s\n", opt.IsSet(), opt.Get().c_str());
	return 0;
}
