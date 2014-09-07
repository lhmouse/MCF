#include <MCF/StdMCF.hpp>
#include <MCF/Containers/MultiIndexMap.hpp>
#include <MCF/Containers/Vlist.hpp>
#include <MCF/Core/String.hpp>
#include <iostream>
using namespace MCF;

template class VList<int>;
template class VList<WideString>;
/*
typedef std::pair<int, std::string> Item;

MULTI_INDEX_MAP(Container, Item,
	UNIQUE_INDEX(first),
	MULTI_INDEX(second),
	SEQUENCED_INDEX()
);
*/
extern "C" unsigned int MCFMain() noexcept {
/*	Container c;
	c.insert(Item(1, "abc"));
	c.insert(Item(2, "def"));
	std::cout <<c.find<0>(1)->second <<std::endl;   // "abc";
	ASSERT(c.upperBound<1>("zzz") == c.end<1>());   // 通过。
*/	return 0;
}
