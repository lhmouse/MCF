#include <MCF/StdMCF.hpp>
#include <MCF/Containers/MultiIndexMap.hpp>
#include <string>
#include <iostream>
#include <cassert>
using namespace MCF;

typedef std::pair<int, std::string> Item;

template class MultiIndexMap<
	Item,
	UniqueOrderedIndex<Item, std::greater<void>>,
	UniqueOrderedMemberIndex<Item, std::string, &Item::second>,
	SequencedIndex
>;

extern "C" unsigned int MCFMain() noexcept {
	MultiIndexMap<
		Item,
		UniqueOrderedIndex<Item, std::greater<void>>,
		UniqueOrderedMemberIndex<Item, std::string, &Item::second>,
		SequencedIndex
	> c, c1;

	c1.Insert(Item(1, "def"));
	c1.Insert(Item(2, "ghi"));
	c1.Insert(Item(3, "abc"));
	c1.Insert(Item(0, "jkl"));
	c = c1;

	__builtin_puts("-----");
//	std::cout <<c.Find<0>(1)->GetElement().second <<std::endl;
	assert(c.GetUpperBound<1>("zzz") == nullptr);
//	auto ret = c.GetEqualRange<0>(2);
//	std::printf("%p %p\n", ret.first, ret.second);
	__builtin_puts("-----");
	for(auto p = c.GetLast<0>(); p; p = p->GetPrev<0>()){
		std::cout <<p->GetElement().first <<", " <<p->GetElement().second <<std::endl;
	}
	__builtin_puts("-----");
//c.SetKey<1>(c.GetLast<0>(), "def");
	for(auto p = c.GetLast<1>(); p; p = p->GetPrev<1>()){
		std::cout <<p->GetElement().first <<", " <<p->GetElement().second <<std::endl;
	}
	__builtin_puts("-----");
	for(auto p = c.GetLast<2>(); p; p = p->GetPrev<2>()){
		std::cout <<p->GetElement().first <<", " <<p->GetElement().second <<std::endl;
	}
	__builtin_puts("-----");

	return 0;
}
