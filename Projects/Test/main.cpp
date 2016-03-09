#include <MCF/StdMCF.hpp>
#include <MCF/Thread/ThreadLocal.hpp>

using namespace MCF;

template class ThreadLocal<int>;
template class ThreadLocal<long double>;

extern "C" unsigned MCFCRT_Main(){
	ThreadLocal<int> tls;
	tls.Set(123);
	return 0;
}
