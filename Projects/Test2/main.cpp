#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
using namespace MCF;

__attribute__((noinline)) void foo(AnsiString &s){
	s.Replace(3, 5, "meow");
}

unsigned int MCFMain(){
	AnsiString s("0123456789");
	foo(s);
	s.Append(s.Slice(1, 10));
	std::printf("%s$\n", s.GetCStr());

	return 0;
}
