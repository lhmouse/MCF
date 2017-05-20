#include <mcfgthread/gthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

__gthread_key_t key;

void tls_destructor(void *p){
	_MCFCRT_ASSERT(__gthread_getspecific(key) == _MCFCRT_NULLPTR);

	printf("destructing tls data %u\n", *(unsigned *)p);
	free(p);
}

void *test_thread_proc(void *param){
	unsigned *p = (unsigned *)malloc(sizeof(unsigned));
	_MCFCRT_ASSERT(p);
	*p = (unsigned)(uintptr_t)param;
	int err = __gthread_setspecific(key, p);
	_MCFCRT_ASSERT(err == 0);
	printf("constructed tls data %u\n", *(unsigned *)p);

	err = __gthread_setspecific(key, p);
	_MCFCRT_ASSERT(err == 0);
	printf("set new tls data %u\n", *(unsigned *)p);

	return _MCFCRT_NULLPTR;
}

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	int err = __gthread_key_create(&key, &tls_destructor);
	_MCFCRT_ASSERT(err == 0);
	printf("key = %p\n", (void *)key);

#define THREAD_COUNT 10u
	__gthread_t threads[THREAD_COUNT];
	for(unsigned i = 0; i < THREAD_COUNT; ++i){
		err = __gthread_create(&threads[i], &test_thread_proc, (void *)(intptr_t)i);
		_MCFCRT_ASSERT(err == 0);
	}
	for(unsigned i = 0; i < THREAD_COUNT; ++i){
		printf("waiting for thread %u\n", i);
		err = __gthread_join(threads[i], _MCFCRT_NULLPTR);
		_MCFCRT_ASSERT(err == 0);
	}

	__gthread_key_delete(key);
	return 0;
}
