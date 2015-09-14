#include <MCF/Utilities/MinMax.hpp>
#include <cstdio>

using namespace MCF;

extern "C" unsigned MCFMain(){
	char a[Min(10, 20, 5)];
	std::printf("size = %zu\n", sizeof(a));
	return 0;
}
