#include <stdlib.h>

int c = 5;

void f(){
	if(--c >= 0){
		atexit(f);
	} else {
		exit(6);
	}
}

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	atexit(f);
	return 0;
}
