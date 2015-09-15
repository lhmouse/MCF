#include <MCF/Containers/List.hpp>
#include <cstdio>

extern "C" unsigned MCFMain(){
	MCF::List<int> l{0,1,2,3,4,5,6,7,8,9};
	for(auto e = l.EnumerateLast(); e; --e){
		std::printf("element = %d\n", *e);
	}
	return 0;
}
