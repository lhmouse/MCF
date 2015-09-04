#include <MCF/StdMCF.hpp>
#include <MCF/Containers/Vector.hpp>

template class MCF::Vector<int>;

extern "C" unsigned MCFMain(){
	MCF::Vector<int> v;
	for(int i = 0; i < 100; ++i){
		v.Push(i);
	}

	std::printf("forward : ");
	for(auto e = v.EnumerateFirst(); e; ++e){
		std::printf("%d ", *e);
	}
	std::puts("");

	std::printf("backward: ");
	for(auto e = v.EnumerateLast(); e; --e){
		std::printf("%d ", *e);
	}
	std::puts("");

	return 0;
}
