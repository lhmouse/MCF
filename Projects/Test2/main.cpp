#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Core/MultiIndexedMap.hpp>

using namespace MCF;

template class MultiIndexedMap<void, MapIndex<MapIndex<AnsiString, std::less<void>>, std::greater<void>>>;

unsigned int MCFMain(){
	MultiIndexedMap<void, MapIndex<MapIndex<AnsiString, std::less<void>>, std::greater<void>>> s;
	s.Insert(AnsiString("acb"));
	s.Insert(AnsiString("cab"));
	auto p = s.Insert(AnsiString("aaa"));
	s.Insert(AnsiString("abc"));
	s.Insert(AnsiString("baa"));
	s.SetIndex<0>(p, AnsiString("bbb"));

	auto begin = s.GetLowerBound<0>("abc");
	auto end = s.GetUpperBound<0>("b");
	for(auto p = begin; p != end; p = p->GetNext<0>()){
		std::printf("%s ", p->GetIndex<0>().GetStr());
	}

	return 0;
}
