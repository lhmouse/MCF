#include <MCF/StdMCF.hpp>
#include <MCF/Utilities/MinMax.hpp>

using namespace MCF;

extern "C" unsigned MCFMain(){
	int temp[Max(1, 2, 3, 4)];
	std::printf("%zu\n", sizeof(temp));
	return 0;
}
