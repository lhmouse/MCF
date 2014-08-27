#include <MCF/StdMCF.hpp>
#include <MCF/Thread/ThreadLocalPtr.hpp>
#include <MCF/Thread/Thread.hpp>
using namespace MCF;

static ThreadLocalPtr<int> g_tls;

extern "C" unsigned int MCFMain() noexcept {
	auto fn = []{
		::Sleep(1000);
		std::printf("thread %lu tls = %d\n", Thread::GetCurrentId(), *g_tls);
		*g_tls = 12345;
		std::printf("thread %lu sets tls to %d\n", Thread::GetCurrentId(), *g_tls);
	};

	auto thread = Thread::Create(fn);
	::Sleep(500);
	fn();
	thread->Join();

	std::printf("child thread has exited, tls = %d\n", *g_tls);

	return 0;
}
