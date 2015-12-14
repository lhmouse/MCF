#include <MCF/StdMCF.hpp>
#include <MCF/Containers/MultiIndexMap.hpp>
#include <MCF/Core/String.hpp>
#include <cstdio>

using namespace MCF;

template class MultiIndexMap<
	MultiMapIndices<
		OrderedMultiIndex<int>,
		OrderedUniqueIndex<char>,
		OrderedUniqueIndex<Utf8String>,
		SequentialIndex>,
	int>;

extern "C" unsigned MCFMain(){
	return 0;
}
