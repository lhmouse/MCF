#include <MCF/StdMCF.hpp>
#include <MCF/Thread/Thread.hpp>
#include <MCF/Thread/ThreadLocal.hpp>

MCF::ThreadLocal<int> tls;

extern "C" unsigned MCFMain(){
	std::printf("thread %u: tls = %d\n", (unsigned)::GetCurrentThreadId(), *tls);
	*tls = 1;
	std::printf("thread %u: tls = %d\n", (unsigned)::GetCurrentThreadId(), *tls);

	auto t = MCF::Thread::Create([]{
		std::printf("child thread %u: tls = %d\n", (unsigned)::GetCurrentThreadId(), *tls);
		*tls = 2;
		std::printf("child thread %u: tls = %d\n", (unsigned)::GetCurrentThreadId(), *tls);
	});
	t->Join();

	return 0;
}
