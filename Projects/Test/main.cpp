#include <mcfgthread/c11thread.h>
#include <stdlib.h>

int thread_proc(void *param){
	__builtin_printf("thread running: tid = %u, param = %p\n", (unsigned)thrd_current(), param);
	for(int i = 0; i < 5; ++i){
		__builtin_printf("thread going to sleep for one second...\n");
		struct timespec ts;
		ts.tv_sec = 1;
		ts.tv_nsec = 0;
		thrd_sleep(&ts, 0);
	}
	int exit_code = 67890;
	__builtin_printf("thread exiting: exit_code = %d\n", exit_code);
	// return exit_code;
	thrd_exit(exit_code);
}

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	thrd_t tid;
	int err;
	if((err = thrd_create(&tid, &thread_proc, (void *)0x12345)) != thrd_success){
		__builtin_printf("thrd_create() returned %d\n", err);
		abort();
	}
	__builtin_printf("created thread: tid = %u\n", (unsigned)tid);
	int exit_code;
	if((err = thrd_join(tid, &exit_code)) != thrd_success){
		__builtin_printf("thrd_join() returned %d\n", err);
		abort();
	}
	__builtin_printf("joined thread: exit_code = %d\n", exit_code);
	return 0;
}
