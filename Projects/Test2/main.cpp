#include <MCF/StdMCF.hpp>
#include <cstdlib>
using namespace MCF;

void foo(){
	throw 123;
}

unsigned int MCFMain(){
	std::atexit(&foo);
	return 0;
}
