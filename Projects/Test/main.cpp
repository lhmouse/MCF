#include <cstdio>
#include <cstdlib>
#include <MCFCRT/env/mcfwin.h>

int t;

void foo(){
	if(t < 10){
		std::atexit(foo);
		std::atexit(foo);
	}

	std::printf("hello! %d\n", ++t);
	::Sleep(100);
}

extern "C" unsigned MCFMain(){
	std::atexit(foo);

	return 0;
}
