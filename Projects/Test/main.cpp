#include <StdMCF.hpp>
#include <Core/Thread.hpp>
#include <cstdio>

void foo(){
	std::puts("foo()");
	throw 12345;
}

unsigned int MCFMain(){
	{
		try {
			MCF::Thread thrd;
			thrd.Start(&foo);
			thrd.Join();

			std::puts("exception uncaught");
		} catch(int e){
			std::printf("exception caught, e = %d\n", e);
		}
	}
	std::puts("return from main");
	return 0;
}
