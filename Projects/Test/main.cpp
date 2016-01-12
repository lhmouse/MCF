#include <MCF/StdMCF.hpp>
#include <MCF/Utilities/ParameterPackManipulators.hpp>

using namespace MCF;

extern "C" unsigned MCFMain(){
	std::printf("first = %zu\n", FindFirstType<int, char, double, int, double, int, char>());
	std::printf("last  = %zu\n", FindLastType <int, char, double, int, double, int, char>());

	return 0;
}
