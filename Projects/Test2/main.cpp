#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Components/Notation.hpp>
using namespace MCF;

AnsiString s("middle ");

__attribute__((noinline)) void foo(){
	s.Replace(-1, -1, "last .............$");
}

unsigned int MCFMain(){
	foo();
	s.Unshift("first ");
	std::puts(s.GetStr());
	return 0;
}
