#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Core/MultiIndexedMap.hpp>
using namespace MCF;

unsigned int MCFMain(){
	MultiIndexedMap<void, AnsiString> s;
	s.Insert("acb");
	s.Insert("cab");
	auto p = s.Insert("aaa");
	s.Insert("abc");
	s.Insert("baa");
	s.SetIndex<0>(p, "bbb");

	auto begin = s.GetBegin<0>();
	auto end = nullptr;
	for(auto p = begin; p != end; p = p->GetNext<0>()){
		std::printf("%s ", p->GetIndex<0>().GetStr());
	}

	return 0;
}
