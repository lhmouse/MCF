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
	const auto p = new auto((unsigned)(intptr_t)param);
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
	std::printf("new TLS key %p\n", (void *)key);
	err = std::atexit([]{ std::printf("deleting TLS key %p\n", (void *)key); ::__gthread_key_delete(key); });
	_MCFCRT_ASSERT(err == 0);

	::__gthread_t threads[10];
	for(auto &t : threads){
		err = ::__gthread_create(&t, &test_thread_proc, (void *)(&t - threads));
		_MCFCRT_ASSERT(err == 0);
	}
	for(auto &t : threads){
		std::printf("waiting for thread %td\n", &t - threads);
		err = ::__gthread_join(t, nullptr);
		_MCFCRT_ASSERT(err == 0);
	}

	return 0;
}
