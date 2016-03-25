#include <MCF/StdMCF.hpp>
#include <MCF/Thread/Thread.hpp>

extern "C" unsigned _MCFCRT_Main(){
	MCF::Thread thread;
	thread.Create([]{ std::puts("thread proc!"); ::Sleep(1000); }, false);
	thread.Join();
	return 0;
}
