#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Thread/Thread.hpp>
#include <MCF/Thread/ThreadLocal.hpp>
#include <cstdio>

using namespace MCF;

namespace {

ThreadLocal<Utf8String> tls;

}

extern "C" {

__declspec(dllexport) int __stdcall dlltest(int a, int b) noexcept {
	std::printf("thread %u: tls = %s\n", (unsigned)::GetCurrentThreadId(), tls.Get().GetStr());
	tls.Set("hello"_u8s);
	std::printf("thread %u: tls = %s\n", (unsigned)::GetCurrentThreadId(), tls.Get().GetStr());

	auto t = Thread::Create([]{
		std::printf("child thread %u: tls = %s\n", (unsigned)::GetCurrentThreadId(), tls.Get().GetStr());
		tls.Set("world"_u8s);
		std::printf("child thread %u: tls = %s\n", (unsigned)::GetCurrentThreadId(), tls.Get().GetStr());
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
