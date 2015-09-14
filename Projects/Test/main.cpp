#include <MCF/Utilities/RationalFunctors.hpp>
#include <cstdlib>
#include <cstdio>

using namespace MCF;

extern "C" unsigned MCFMain(){
	void *p = std::malloc(200);
	p = std::realloc(p, 100);
	// free(p);
	return 0;
}
