#include <MCF/Containers/Vector.hpp>

using namespace MCF;

extern "C" unsigned MCFMain(){
	Vector<int> v{0,1,2,3,4,5,6,7,8,9};
	for(auto e = v.EnumerateFirst(); e; ++e){
		std::printf("element: %d\n", *e);
	}
	return 0;
}
