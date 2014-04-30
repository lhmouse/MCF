#include <MCF/StdMCF.hpp>
#include <MCF/Core/ThreadLocal.hpp>
using namespace MCF;

struct S {
	S(){
		throw 12345;
	}
	void foo(){
	}
};

ThreadLocal<S> tls;

unsigned int MCFMain(){
	try {
		tls->foo();
	} catch(int e){
		std::printf("exception %d\n", e);
	}
	return 0;
}
