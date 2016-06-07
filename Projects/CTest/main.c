#include <MCFCRT/env/gthread.h>
#include <MCFCRT/env/clocks.h>
#include <stdio.h>

void *thread_proc(void *param){
	printf("thread - 1: param = %p\n", param);
	_MCFCRT_Sleep(_MCFCRT_GetFastMonoClock() + 1000);
	printf("thread - 2: param = %p\n", param);
	return (void *)0x54321;
}

unsigned int _MCFCRT_Main(void){
	__gthread_t tid;
	__gthread_create(&tid, &thread_proc, (void *)0x12345);
	printf("main - 1: tid = %u\n", (unsigned)tid);
	void *ret;
	__gthread_join(tid, &ret);
	_MCFCRT_Sleep(_MCFCRT_GetFastMonoClock() + 2000);
	printf("main - 2: ret = %p\n", ret);
	return 0;
}
