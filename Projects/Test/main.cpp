#include <MCF/Containers/FlatMultiSet.hpp>
#include <cstdio>

template class MCF::FlatMultiSet<int>;

extern "C" unsigned MCFMain(){
	MCF::FlatMultiSet<int> set{6,7,8,1,0,2,9,4,3,5};
	set.AddUsingHint(set.GetBegin() + 1, 1);
	set.AddUsingHint(set.GetBegin() + 2, 6);
	set.AddUsingHint(set.GetBegin() + 3, 2);
	set.AddUsingHint(set.GetBegin() + 4, 6);
	set.AddUsingHint(set.GetBegin() + 5, 3);
	set.AddUsingHint(set.GetBegin() + 6, 6);
	set.AddUsingHint(set.GetBegin() + 7, 4);
	auto r = set.EnumerateEqualRange(6);
	for(auto e = r.first; e != r.second; ++e){
		std::printf("element: %d\n", *e);
	}
	return 0;
}
