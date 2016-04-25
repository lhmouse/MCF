#include <MCF/StdMCF.hpp>
#include <MCF/Thread/ThreadLocal.hpp>
#include <cstdio>

template class MCF::ThreadLocal<int>;

extern "C" unsigned _MCFCRT_Main(){
	try {
		throw 123;
	} catch(int e){
		std::printf("exception caught: e = %d\n", e);
	}
	return 0;
}
