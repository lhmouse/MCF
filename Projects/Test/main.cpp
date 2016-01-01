#include <MCF/StdMCF.hpp>
#include <MCF/Core/Array.hpp>

using namespace MCF;

template class Array<int, 2, 3>;

extern "C" unsigned MCFMain(){
	Array<int, 2, 3> a;
	a.Get(0) = { 1, 2, 3 };
	a.Get(1) = { 4, 5, 6 };

	for(unsigned i = 0; i < 2; ++i){
		for(unsigned j = 0; j < 3; ++j){
			std::printf("a[%u][%u] = %d\n", i, j, a[i][j]);
		}
	}

	try {
		a.Get(2); // out of range
	} catch(std::exception &e){
		std::printf("exception: %s\n", e.what());
	}

	return 0;
}
