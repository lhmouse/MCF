#include <MCF/StdMCF.hpp>
#include <MCF/Core/Clocks.hpp>
#include <MCF/Thread/Thread.hpp>
/*
extern "C" void _MCFCRT_OnCtrlEvent(bool bIsSigInt){
	std::printf("Ctrl event: bIsSigInt = %d\n", bIsSigInt);
}
*/
extern "C" unsigned _MCFCRT_Main(){
	std::printf("Waiting...\n");
	MCF::Thread::Sleep(MCF::GetFastMonoClock() + 5000);
	return 0;
}
