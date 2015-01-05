#include <MCF/StdMCF.hpp>
#include <MCF/Containers/MultiIndexMap.hpp>
using namespace MCF;

using element = std::pair<int, int>;

template class MultiIndexMap<element,
	UniqueOrderedMemberIndex<element, int, &element::first>
	>;

extern "C" unsigned int MCFMain() noexcept {
	MultiIndexMap<element,
		UniqueOrderedMemberIndex<element, int, &element::first>
		> m;

	m.Insert(false, 3, 9);
	m.Insert(false, 2, 6);
	m.Insert(false, 1, 3);

	auto cur = m.GetFirstCursor<0>();
	while(cur){
		std::printf("%d => %d\n", cur->first, cur->second);
		++cur;
	}

	return 0;
}
