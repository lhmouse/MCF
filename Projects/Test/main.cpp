#include <MCF/Core/Thread.hpp>
#include <MCFCRT/MCFCRT.h>
#include <cstdio>
#include <cstdlib>

static MCF::TheadLocal<int> a;

void foo(){
	*a = 67890;
	std::printf("thread: a = %d\n", *a);
}

unsigned int MCFMain(){
	*a = 12345;
	std::printf("main  : a = %d\n", *a);
	MCF::Thread thrd;
	thrd.Start(foo);
	thrd.Join();
	std::printf("main  : a = %d\n", *a);
	return 0;
}
