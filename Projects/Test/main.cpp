#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Containers/MultiIndexMap.hpp>
using namespace MCF;

template class MultiIndexMap<Utf8String,
	UniqueOrderedIndex<Utf8String>
	>;

extern "C" unsigned int MCFMain() noexcept {
	MultiIndexMap<Utf8String,
		UniqueOrderedIndex<Utf8String>
		> set;
	set.Insert(false, "hhhh");
	set.Insert(false, "cccccc");
	set.Insert(false, "ddddddddd");
	set.Insert(false, "zzzzz");
	set.Insert(false, "ccdd");
	set.Insert(false, "aaaaa");
	set.Insert(false, "xxxxxx");
	set.Insert(false, "ccbb");
	for(auto c = set.GetFirstCursor<0>(); c; ++c){
		std::puts(c->GetStr());
	}
	return 0;
}
