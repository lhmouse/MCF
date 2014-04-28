#include <MCF/StdMCF.hpp>
#include <MCF/Language/TExpression.hpp>
using namespace MCF;

unsigned int MCFMain(){
	try {
		TExpression texp;
		auto result = texp.Parse(
			L"mul( \n"
			"  add(1 2 3) \n"
			"  sub(4 5) \n"
			")"
			"not \n"
			"div( \n"
			"  and(6 7 8) \n"
			"  neg(9) \n"
			") \n"
			"() \n"
		);
		std::printf("%d\n------------\n%ls", result.first, texp.Export().GetCStr());
		throw 1;
	} catch(int e){
		std::printf("exception %d\n", e);
	}
	return 0;
}
