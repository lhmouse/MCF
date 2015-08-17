#include <MCF/StdMCF.hpp>
#include <MCF/Thread/Atomic.hpp>

using namespace MCF;

template class Impl_Atomic::Atomic<int, 1, 0>;
template class Impl_Atomic::Atomic<int *, 0, 1>;

extern "C" unsigned MCFMain(){
	return 0;
}
