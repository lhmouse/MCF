#include <MCF/StdMCF.hpp>
#include <MCF/Containers/FlatMap.hpp>
#include <MCF/Containers/FlatMultiMap.hpp>
#include <MCF/Containers/FlatMultiSet.hpp>
#include <MCF/Containers/FlatSet.hpp>
#include <MCF/Containers/List.hpp>
#include <MCF/Containers/RingQueue.hpp>
#include <MCF/Containers/StaticVector.hpp>
#include <MCF/Containers/Vector.hpp>

using namespace MCF;

template class FlatMap<int, unsigned>;
template class FlatMultiMap<int, unsigned>;
template class FlatMultiSet<int>;
template class FlatSet<int>;
template class List<int>;
template class RingQueue<int>;
template class StaticVector<int, 5>;
template class Vector<int>;

template void MCF::swap(FlatMap<int, unsigned> &,             FlatMap<int, unsigned> &);
template void MCF::swap(FlatMultiMap<int, unsigned> &,        FlatMultiMap<int, unsigned> &);
template void MCF::swap(FlatMultiSet<int> &,                  FlatMultiSet<int> &);
template void MCF::swap(FlatSet<int> &,                       FlatSet<int> &);
template void MCF::swap(List<int> &,                          List<int> &);
template void MCF::swap(RingQueue<int> &,                     RingQueue<int> &);
template void MCF::swap(StaticVector<int, 5> &,               StaticVector<int, 5> &);
template void MCF::swap(Vector<int> &,                        Vector<int> &);

extern "C" unsigned MCFMain(){
	return 0;
}
