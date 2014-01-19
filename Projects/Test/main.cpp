#include <StdMCF.hpp>
#include <Core/Thread.hpp>
#include <cstdio>
#include <cstdlib>

unsigned int MCFMain(){
	MCF::Thread thrd;
	thrd.Start([]{
		std::puts("in thread");
		throw 12345;
	});
	try {
		thrd.Join();
		std::puts("no exception caught");
	} catch(int e){
		std::printf("exception caught, e = %d\n", e);
	}
	return 0;
}
