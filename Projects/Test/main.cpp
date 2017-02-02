#include <cstdlib>
#include <cstdio>

static void foo(){
	thread_local char a;
	std::printf("%p\n", &a);
}

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	std::atexit(&foo);
	return 0;
}
