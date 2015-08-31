#include <MCF/StdMCF.hpp>
#include <MCF/Core/StringObserver.hpp>
#include <MCF/StdMCF.hpp>
#include <MCF/Thread/Thread.hpp>
#include <MCF/Thread/ThreadLocal.hpp>
#include <cstdio>

using namespace MCF;

namespace {

ThreadLocal<int> tls;

}

extern "C" {

__declspec(dllexport) int __stdcall dlltest(int a, int b) noexcept {
	std::printf("thread %u: tls = %d\n", (unsigned)::GetCurrentThreadId(), *tls);
	*tls = 1;
	std::printf("thread %u: tls = %d\n", (unsigned)::GetCurrentThreadId(), *tls);

	auto t = MCF::Thread::Create([]{
		std::printf("child thread %u: tls = %d\n", (unsigned)::GetCurrentThreadId(), *tls);
		*tls = 2;
		std::printf("child thread %u: tls = %d\n", (unsigned)::GetCurrentThreadId(), *tls);
	});
	t->Join();

	return a + b;
}

bool MCFDll_OnProcessAttach(void *hDll, bool bDynamic) noexcept {
	std::printf("on process attach: hDll = %p, dynamic = %d\n", hDll, bDynamic);
	return true;
}
void MCFDll_OnProcessDetach(void *hDll, bool bDynamic) noexcept {
	std::printf("on process detach: hDll = %p, dynamic = %d\n", hDll, bDynamic);
}

void MCFDll_OnThreadAttach(void *hDll) noexcept {
	std::printf("on thread attach: hDll = %p\n", hDll);
}
void MCFDll_OnThreadDetach(void *hDll) noexcept {
	std::printf("on thread detach: hDll = %p\n", hDll);
}

}
