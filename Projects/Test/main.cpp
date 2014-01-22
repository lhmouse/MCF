#include <MCFCRT/MCFCRT.h>
#include <MCF/Core/Thread.hpp>
#include <cstdio>
#include <cstdlib>

THREAD_LOCAL(int) i = 9;

unsigned int MCFMain(){
	MCF::Thread thrd;
	thrd.Start([&]{
		*i = 200;
		std::printf("i = %d\n", *i);
	});
	thrd.Join();
	std::printf("i = %d\n", *i);
	return 0;
}
