#include <MCF/Core/Optional.hpp>
#include <string>
#include <cstdio>

using namespace MCF;

template class Optional<std::string>;

extern "C" unsigned MCFMain(){
	Optional<std::string> opt;

	opt.ResetElement("hello world!");
	std::printf("set = %d, exception set = %d\n", opt.IsSet(), opt.IsExceptionSet());
	std::printf("str = %s\n", opt.Get().c_str());

	opt.ResetException(std::make_exception_ptr(123));
	try {
		std::printf("set = %d, exception set = %d\n", opt.IsSet(), opt.IsExceptionSet());
		std::printf("str = %s\n", opt.Get().c_str());
	} catch(int e){
		std::printf("caught exception: e = %d\n", e);
	}

	return 0;
}
