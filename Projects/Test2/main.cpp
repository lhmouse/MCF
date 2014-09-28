#include <MCF/StdMCF.hpp>
#include <MCF/Containers/MultiIndexMap.hpp>
#include <MCF/Core/String.hpp>
using namespace MCF;

struct Item {
	int i;
	AnsiString s;

	Item(int i_, AnsiString s_)
		: i(i_), s(std::move(s_))
	{
	}
};

extern "C" unsigned int MCFMain() noexcept {
	MultiIndexMap<Item,
		UniqueOrderedMemberIndex<Item, int, &Item::i>,
		UniqueOrderedMemberIndex<Item, AnsiString, &Item::s>
	> m;

	const auto Print = [&]{
		std::printf("--- size = %u\n", (unsigned)m.GetSize());
		for(auto p = m.GetFirst<0>(); p; p = p->GetNext<0>()){
			std::printf("%d, %s\n", p->i, p->s.GetCStr());
		}
	};

	m.Insert(false, Item(3, "ghi"_as));
	auto p = m.Insert(false, Item(2, "def"_as)).first;
	m.Insert(false, Item(1, "abc"_as));
	Print();

	m.SetKey<0>(true, p, 3);
	Print();

	return 0;
}
