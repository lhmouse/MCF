#include <MCF/StdMCF.hpp>
#include <MCF/Thread/Atomic.hpp>

using namespace MCF;

struct foo {
	int a;
};

enum bar {
	v0, v1, v2
};

template class Atomic<bool>;
template class Atomic<int>;
template class Atomic<foo>;
template class Atomic<foo *>;
template class Atomic<void *>;
template class Atomic<void (*)(int)>;
template class Atomic<bar>;

extern "C" unsigned MCFCRT_Main(){
	Atomic<int> a;
	a.Increment(kAtomicConsume);
	return 0;
}
