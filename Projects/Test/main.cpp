#include <MCF/StdMCF.hpp>
#include <MCF/Containers/MultiIndexMap.hpp>
using namespace MCF;

struct element {
	int a, b, c;

	element(int a_, int b_, int c_) noexcept
		: a(a_), b(b_), c(c_)
	{
	}
};

template class MultiIndexMap<element,
	UniqueOrderedMemberIndex<element, int, &element::a>,
	UniqueOrderedMemberIndex<element, int, &element::b>,
	UniqueOrderedMemberIndex<element, int, &element::c>
	>;

extern "C" unsigned int MCFMain() noexcept {
	MultiIndexMap<element,
		UniqueOrderedMemberIndex<element, int, &element::a>,
		UniqueOrderedMemberIndex<element, int, &element::b>,
		UniqueOrderedMemberIndex<element, int, &element::c>
		> map;

		map.Insert(false, 4, 5, 8);
	auto k =
		map.Insert(false, 2, 4, 6);
		map.Insert(false, 3, 6, 9);
		map.Insert(false, 1, 2, 3);

	map.Replace(true, k.first, 3, 5, 6);
	for(auto p = map.GetFirst<0>(); p; p = p->GetNext<0>()){
		std::printf("%d %d %d\n", p->a, p->b, p->c);
	}

	return 0;
}
