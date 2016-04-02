#include <MCF/StdMCF.hpp>
#include <MCF/Thread/Thread.hpp>
#include <MCF/Thread/ThreadLocal.hpp>
#include <cstdio>

extern "C" unsigned _MCFCRT_Main(){
	auto l = [](std::intptr_t s){ std::printf("at thread exit: s = %s\n", (const char *)s); };
	::_MCFCRT_AtThreadExit(l, (std::intptr_t)"abc");
	::_MCFCRT_AtThreadExit(l, (std::intptr_t)"defg");
	::_MCFCRT_AtThreadExit(l, (std::intptr_t)"hijkl");

	MCF::Thread t(
		[&]{
			std::printf("in thread: %zu\n", MCF::Thread::GetCurrentId());
			::_MCFCRT_AtThreadExit(l, (std::intptr_t)"xyz");
		},
		false);
	t.Join();

	return 0;
}
