#include <stdio.h>
#include <stdlib.h>

__attribute__((__constructor__))
static void ctor(void){
	puts("ctor!");
}
__attribute__((__destructor__))
static void dtor(void){
	puts("dtor!");
}

static void cleanup(void){
	puts("cleanup!");
}

unsigned int _MCFCRT_Main(void){
	puts("meow!");
	atexit(&cleanup);
	return 0;
}
