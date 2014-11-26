#include <MCF/StdMCF.hpp>
#include <MCF/Containers/MultiIndexMap.hpp>
#include <utility>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	MultiIndexMap<std::pair<int, int>,
		UniqueOrderedMemberIndex<std::pair<int, int>, int, &std::pair<int, int>::first>> map;
	map.Insert(true, std::make_pair(1, 2));
	map.Insert(true, std::make_pair(2, 3));
	map.Insert(true, std::make_pair(1, 4));
	for(auto p = map.GetFirst<0>(); p; p = p->GetNext<0>()){
		std::printf("%d %d\n", p->first, p->second);
	}
	return 0;
}
