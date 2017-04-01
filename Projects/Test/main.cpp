#include <MCFCRT/env/_mopthread.h>
#include <MCFCRT/env/thread.h>
#include <MCFCRT/env/clocks.h>
#include <MCFCRT/env/heap.h>
#include <cstdio>

static void thread_proc(void *param) noexcept {
	long long n;
	__builtin_memcpy(&n, param, sizeof(n));
	std::printf("thread running: n = %lld\n", n);
	::__MCFCRT_MopthreadExit(nullptr, 0);
}

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	long long n = 0x0123456789abcdef;
	const std::size_t tid = ::__MCFCRT_MopthreadCreateDetached(thread_proc, &n, sizeof(n));
	std::printf("spawned thread: tid = %zu\n", tid);
	const auto ph = ::__MCFCRT_MopthreadLockHandle(tid);
	std::printf("ph = %p\n", (void *)ph);
	::_MCFCRT_Sleep(::_MCFCRT_GetFastMonoClock() + 1000);
	return 0;
}
