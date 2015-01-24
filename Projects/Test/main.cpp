#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Containers/MultiIndexMap.hpp>
using namespace MCF;

struct Element {
	Utf8String key;
	int val;
};

template class MultiIndexMap<Element,
	UniqueOrderedMemberIndex<Element, Utf8String, &Element::key>
	>;

extern "C" unsigned int MCFMain() noexcept {
	MultiIndexMap<Element,
		UniqueOrderedMemberIndex<Element, Utf8String, &Element::key>
		> m;

	Element e{ "meow"_u8s, 123 };
	m.Insert(true, e);
	m.Insert(true, e);
	m.Insert(true, e);

	return 0;
}
