#include <MCF/StdMCF.hpp>
#include <MCF/Utilities/MinMax.hpp>

using namespace MCF;

extern "C" unsigned MCFMain(){
	std::printf("max = %f\n", Max(2,5l,6ll,1.0f,'\x8',4,3));

	return 0;
}
