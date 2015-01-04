#include <MCF/StdMCF.hpp>
#include <MCF/Containers/Deque.hpp>
using namespace MCF;

template class Deque<int>;

extern "C" unsigned int MCFMain() noexcept {
	Deque<int> q;
	q.AppendFill(1000);
	return 0;
}
