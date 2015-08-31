#include <MCF/StdMCF.hpp>
#include <MCF/Thread/Thread.hpp>
#include <MCF/Thread/ThreadLocal.hpp>
#include <MCF/Core/String.hpp>

MCF::ThreadLocal<int> tls;

extern "C" unsigned MCFMain(){
	std::printf("thread %u: tls = %d\n", (unsigned)::GetCurrentThreadId(), tls.Get());
	tls.Set(1);
	std::printf("thread %u: tls = %d\n", (unsigned)::GetCurrentThreadId(), tls.Get());

	auto t = MCF::Thread::Create([]{
		std::printf("child thread %u: tls = %d\n", (unsigned)::GetCurrentThreadId(), tls.Get());
		tls.Set(2);
		std::printf("child thread %u: tls = %d\n", (unsigned)::GetCurrentThreadId(), tls.Get());
	});
	t->Join();

	return 0;
}
