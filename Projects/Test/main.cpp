#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>

using namespace MCF;

__attribute__((__noinline__))
void replace(Utf8String &s){
	s.Replace(2, 2, s);
}

extern "C" unsigned MCFMain(){
	Utf8String s("hello world!");
	for(unsigned i = 0; i < 10; ++i){
		replace(s);
		std::puts(s.GetStr());
	}
	return 0;
}
