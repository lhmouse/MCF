#include <MCF/Containers/FlatSet.hpp>
#include <cstdio>

template class MCF::FlatSet<int>;

extern "C" unsigned MCFMain(){
	MCF::FlatSet<int> set{6,7,8,1,0,2,9,4,3,5};
	set.AddUsingHint(set.GetBegin() + 1, 5);
	set.AddUsingHint(set.GetBegin() + 2, 5);
	set.AddUsingHint(set.GetBegin() + 3, 5);
	set.AddUsingHint(set.GetBegin() + 4, 5);
	set.AddUsingHint(set.GetBegin() + 5, 5);
	set.AddUsingHint(set.GetBegin() + 6, 5);
	set.AddUsingHint(set.GetBegin() + 7, 5);
	for(auto e = set.EnumerateLowerBound(3); e; ++e){
		std::printf("element: %d\n", *e);
	}
	return 0;
}
