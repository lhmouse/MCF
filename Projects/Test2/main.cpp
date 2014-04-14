#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
using namespace MCF;

unsigned int MCFMain(){
	AnsiString a("hello "), b("world! ");
	a += b;
	std::puts(a.GetCStr());

	std::printf("%d\n", a == "hello world! ");

	b.Unshift(a);
	std::puts(b.GetCStr());

	return 0;
}
