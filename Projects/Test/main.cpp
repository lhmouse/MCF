#include <MCF/StdMCF.hpp>
#include <MCFCRT/env/thread.h>

extern "C" unsigned _MCFCRT_Main(){
	::_MCFCRT_AtThreadExit([](int n){ std::printf("n = %d\n", n); }, 12345);
	return 0;
}
