#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
using namespace MCF;

unsigned int MCFMain(){
	AnsiString s("world!    $");
	s.Unshift("hello ");
	std::puts(s.GetCStr());
	return 0;
}
