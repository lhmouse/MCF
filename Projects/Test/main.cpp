#include <MCF/Containers/FlatMap.hpp>
#include <cstdio>

template class MCF::FlatMap<int, unsigned>;

extern "C" unsigned MCFMain(){
	MCF::FlatMap<int, unsigned> m;
	m.Reserve(8);
	m.Add    (5, 99u);
	m.Add    (3,  9u);
	m.Add    (4, 16u);
	m.Add    (1,  1u);
	m.Add    (6, 36u);
	m.Add    (2,  4u);
//	m.Add    (5, 25u);
	m.Replace(5, 25u);
	for(auto e = m.EnumerateFirst(); e; ++e){
		std::printf("element: %d => %u\n", e->first, e->second);
	}
	return 0;
}
