#include <MCF/StdMCF.hpp>
#include <cstdio>
#include <MCF/Core/String.hpp>
#include <MCF/Core/Bind.hpp>
#include <MCF/Thread/Thread.hpp>
#include <MCF/Thread/ThreadLocal.hpp>

using namespace MCF;

namespace {

ThreadLocal<Utf8String> tls;

inline Utf8String &GetString(){
	return *tls.Require();
}

}

extern "C" {

int __stdcall dlltest(int a, int b) noexcept {
	auto f = [](Utf8String s){
		std::printf("thread %u: tls = %s\n", (unsigned)::GetCurrentThreadId(), GetString().GetStr());
		GetString() = std::move(s);
		std::printf("thread %u: tls = %s\n", (unsigned)::GetCurrentThreadId(), GetString().GetStr());
	};

	Thread t;
	t.Create(Bind(f, "world"_u8s), false);
	f("hello"_u8s);
	t.Join();

	return a + b;
}

bool _MCFCRT_OnDllProcessAttach(void *hDll, bool bDynamic) noexcept {
	std::printf("on process attach: hDll = %p, dynamic = %d\n", hDll, bDynamic);
	return true;
}
void _MCFCRT_OnDllProcessDetach(void *hDll, bool bDynamic) noexcept {
	std::printf("on process detach: hDll = %p, dynamic = %d\n", hDll, bDynamic);
}

void _MCFCRT_OnDllThreadAttach(void *hDll) noexcept {
	std::printf("on thread attach: hDll = %p\n", hDll);
}
void _MCFCRT_OnDllThreadDetach(void *hDll) noexcept {
	std::printf("on thread detach: hDll = %p\n", hDll);
}

}
