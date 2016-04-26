#include <MCF/StdMCF.hpp>
#include <MCF/Thread/ThreadLocal.hpp>
#include <cstring>
#include <cstdio>
#include <mcfgthread/gthread.h>

struct foo {
	char large[100];
	foo(){
		std::strcpy(large, "hello world!");
		std::printf("ctor of %p\n", (void *)this);
	}
	~foo(){
		std::printf("dtor of %p\n", (void *)this);
	}
	void bark(){
		std::printf("large = %s\n", large);
	}
};

template class MCF::ThreadLocal<foo>;

MCF::ThreadLocal<foo> zz;

__MCFCRT_C_STDCALL
unsigned long test_thread_proc(void *){
	zz.Require()->bark();
	return 0;
}
#define THREAD_COUNT 3
extern "C" unsigned _MCFCRT_Main(){
	_MCFCRT_ThreadHandle threads[THREAD_COUNT];
	for(unsigned i = 0; i < THREAD_COUNT; ++i){
		_MCFCRT_ThreadHandle h = _MCFCRT_CreateNativeThread(&test_thread_proc, nullptr, false, nullptr);
		assert(h);
		threads[i] = h;
	}
	for(unsigned i = 0; i < THREAD_COUNT; ++i){
		_MCFCRT_ThreadHandle h = threads[i];
		printf("waiting for thread %u\n", i);
		_MCFCRT_WaitForThreadForever(h);
		_MCFCRT_CloseThread(h);
	}
	return 0;
}
