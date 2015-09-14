#include <MCF/Utilities/RationalFunctors.hpp>
#include <cstdio>

using namespace MCF;

extern "C" unsigned MCFMain(){
	int i, j;
	std::printf("r = %d\n", EqualTo      ()(&i, &j));
	std::printf("r = %d\n", NotEqualTo   ()(&i, &j));
	std::printf("r = %d\n", Less         ()(&i, &j));
	std::printf("r = %d\n", Greater      ()(&i, &j));
	std::printf("r = %d\n", LessEqual    ()(&i, &j));
	std::printf("r = %d\n", GreaterEqual ()(&i, &j));
	return 0;
}
