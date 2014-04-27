#include <MCF/StdMCF.hpp>
#include <MCF/Components/TExpression.hpp>
#include <MCF/Components/Notation.hpp>
using namespace MCF;

unsigned int MCFMain(){
	WideString ws(L"#include <MCF/Components/TExpression.hpp>");
	std::printf("%zu\n", ws.Find(L'T'));
	return 0;
}
