#include <MCF/StdMCF.hpp>
#include <MCF/Thread/ThreadLocal.hpp>
#include <MCF/SmartPointers/UniquePtr.hpp>
#include <cstdio>

struct foo {
	MCF::UniquePtr<int> p;

	foo()
		: p(new int)
	{
		std::printf("foo::foo(), this = %p\n", (void *)this);
//		throw 12345;
	}
	void bark(){
		std::printf("foo::bark(), this = %p\n", (void *)this);
	}
};

template class MCF::ThreadLocal<foo>;

extern "C" unsigned _MCFCRT_Main(){
	MCF::ThreadLocal<foo> tls;
	try {
		tls.Open()->bark();
	} catch(int e){
		std::printf("exception caught: e = %d\n", e);
	}
	return 0;
}
