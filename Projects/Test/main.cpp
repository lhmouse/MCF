#include <MCF/StdMCF.hpp>
#include <MCF/Thread/ThreadLocal.hpp>

template class MCF::ThreadLocal<int>;

extern "C" unsigned _MCFCRT_Main(){
	return 0;
}
