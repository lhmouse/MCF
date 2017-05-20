#include <mcfgthread/gthread.h>
#include <cstdio>
#include <cstdlib>

::__gthread_key_t key;

void tls_destructor(void *p){
	_MCFCRT_ASSERT(::__gthread_getspecific(key) == nullptr);
	std::printf("destructing tls data %u\n", *(unsigned *)p);
	delete (unsigned *)p;
}

void *test_thread_proc(void *param){
	const auto p = new unsigned((unsigned)(uintptr_t)param);
	int err = ::__gthread_setspecific(key, p);
	_MCFCRT_ASSERT(err == 0);
	std::printf("constructed tls data %u\n", *(unsigned *)p);

	err = ::__gthread_setspecific(key, p);
	_MCFCRT_ASSERT(err == 0);
	std::printf("set new tls data %u\n", *(unsigned *)p);

	return nullptr;
}

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	int err = ::__gthread_key_create(&key, &tls_destructor);
	_MCFCRT_ASSERT(err == 0);
	std::printf("key = %p\n", (void *)key);
	err = std::atexit([]{ ::__gthread_key_delete(key); });
	_MCFCRT_ASSERT(err == 0);

	static constexpr unsigned n = 10;
	__gthread_t threads[n];
	for(unsigned i = 0; i < n; ++i){
		err = ::__gthread_create(&threads[i], &test_thread_proc, (void *)(intptr_t)i);
		_MCFCRT_ASSERT(err == 0);
	}
	for(unsigned i = 0; i < n; ++i){
		std::printf("waiting for thread %u\n", i);
		err = ::__gthread_join(threads[i], nullptr);
		_MCFCRT_ASSERT(err == 0);
	}

	return 0;
}
