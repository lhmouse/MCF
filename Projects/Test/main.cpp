#include <MCF/StdMCF.hpp>
#include <MCF/Utilities/Bail.hpp>

extern "C" unsigned MCFMain(){
	MCF::BailF(L"aa %d %f", 123, 45.6);
	return 0;
}
