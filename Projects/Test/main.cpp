#include <MCF/Containers/FlatSet.hpp>
#include <MCF/Containers/FlatMultiSet.hpp>
#include <cstdio>

template class MCF::FlatSet<int>;
template class MCF::FlatMultiSet<int>;

extern "C" unsigned MCFMain(){
	return 0;
}
