#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <cstdio>

using namespace MCF;

__attribute__((__noinline__))
void foo(Utf8String &s){
	auto p = s.GetStr();
	std::puts(p);
}

extern "C" unsigned int MCFMain() noexcept {
	Utf8String s('a', 50);
	foo(s);
	return 0;
}
