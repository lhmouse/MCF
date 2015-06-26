#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <cstdio>

using namespace MCF;

__attribute__((__noinline__))
void foo(Utf8String &s, std::size_t c){
	s.Resize(c);
}

extern "C" unsigned int MCFMain() noexcept {
	Utf8String s;
	foo(s, 500);
	return 0;
}
