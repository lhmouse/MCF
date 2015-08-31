#include <MCF/StdMCF.hpp>

extern "C" unsigned MCFMain(){
	(new int)[0] = 0x12345678;

	return 0;
}
