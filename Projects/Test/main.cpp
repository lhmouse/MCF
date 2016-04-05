#include <MCF/StdMCF.hpp>
#include <MCF/Thread/ThreadLocal.hpp>

template class MCF::ThreadLocal<int>;
template class MCF::ThreadLocal<long double>;

extern "C" unsigned _MCFCRT_Main(){
	auto l = [](std::intptr_t n){ __builtin_printf("at thread exit: %d\n", (int)n); };
	::_MCFCRT_AtThreadExit(l, 1);
	::_MCFCRT_AtThreadExit(l, 2);
	::_MCFCRT_AtThreadExit(l, 3);
	return 0;
}
