#include <MCF/StdMCF.hpp>
#include <MCF/Thread/KernelMutex.hpp>

using namespace MCF;

extern "C" unsigned _MCFCRT_Main(){
	KernelMutex m(L"this_is_a_very_loooooooooooooooong_name"_wsv, 0);
	return 0;
}
