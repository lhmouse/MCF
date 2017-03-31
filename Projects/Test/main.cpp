#include <MCFCRT/env/_mopthread.h>
#include <MCFCRT/env/thread.h>
#include <MCFCRT/env/clocks.h>
#include <MCFCRT/env/heap.h>
#include <cstdio>

static void thread_proc(void *param) noexcept {
	int n;
	__builtin_memcpy(&n, param, sizeof(n));
	std::printf("thread running: n = %d\n", n);
}

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	int n = 12345;
	const std::size_t tid = ::__MCFCRT_MopthreadCreateDetached(thread_proc, &n, sizeof(n));
	std::printf("spawned thread: tid = %zu\n", tid);
	const auto ph = ::__MCFCRT_MopthreadLockHandle(tid);
	std::printf("ph = %p\n", (void *)ph);
	::_MCFCRT_Sleep(::_MCFCRT_GetFastMonoClock() + 1000);
	return 0;
}
