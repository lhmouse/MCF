#include <MCF/StdMCF.hpp>
#include <MCF/Components/TExpression.hpp>
#include <MCF/Components/Notation.hpp>
using namespace MCF;

unsigned int MCFMain(){
	TExpression texp;
	auto result = texp.Parse(
		L"\\x20mul\\x9\\ (\n"
		L"  add(1 2 3)\n"
		L"  sub(\"9\\x41 \\\"6\"7 8)\n"
		L") ; comment \\\n"
		L"    more comments \n"
		L"()\n"
		L"meow (4 5)\n"
		L"1(2)\n"
	);
	std::printf("result = %d\n------------\n%ls", result.first, texp.Export().GetCStr());
	return 0;
}
