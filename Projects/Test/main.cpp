#include <MCF/Containers/StaticVector.hpp>
#include <cstdio>

using namespace MCF;

template class StaticVector<int, 8>;

extern "C" unsigned MCFMain(){
	StaticVector<int, 8> a{0,1,2,3,4,5,6};
	for(auto e : a){
		std::printf("e = %d\n", e);
	}
	return 0;
}
