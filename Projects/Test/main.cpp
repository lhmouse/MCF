#include <MCF/StdMCF.hpp>
#include <cstdio>
//#include <MCFCRT/env/thread.h>
#include <MCF/Thread/Thread.hpp>

extern "C" unsigned MCFCRT_Main(){
/*
	auto h = ::MCFCRT_CreateThread(
		[](std::intptr_t) noexcept -> unsigned {
			std::printf("child: current thread id = %u\n", (unsigned)::MCFCRT_GetCurrentThreadId());
			return 0;
		},
		12345, false, nullptr);
	std::printf("main : current thread id = %u\n", (unsigned)::MCFCRT_GetCurrentThreadId());
	::MCFCRT_WaitForThreadInfinitely(h);
	::MCFCRT_CloseThread(h);
*/
	auto p = MCF::Thread::Create(
		[]{
			std::printf("child: current thread id = %u\n", (unsigned)MCF::Thread::GetCurrentId());
		}, false);
	std::printf("main : current thread id = %u\n", (unsigned)MCF::Thread::GetCurrentId());
	p->Wait();

	return 0;
}
