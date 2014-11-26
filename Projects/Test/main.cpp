#include <MCF/StdMCF.hpp>
#include <MCF/Containers/MultiIndexMap.hpp>
#include <utility>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	using Element = std::pair<int, int>;
	MultiIndexMap<Element,
		UniqueOrderedMemberIndex<Element, int, &Element::first>
		> map;
	map.Insert(true, std::make_pair(1, 2));
	map.Insert(true, std::make_pair(2, 3));
	map.Insert(true, std::make_pair(1, 4));
// __debugbreak();
	for(auto p = map.GetFirst<0>(); p; p = p->GetNext<0>()){
		std::printf("%d %d\n", p->first, p->second);
	}
	return 0;
}
