#include <MCF/StdMCF.hpp>
#include <cstdio>

struct foo {
	explicit foo(const char *s){
		std::printf("ctor: this = %p, str = %s\n", (void *)this, s);
	}
	~foo(){
		std::printf("dtor: this = %p\n", (void *)this);
	}
};

extern "C" unsigned _MCFCRT_Main(){
	static foo f1("static");
	thread_local foo f2("thread_local");
	std::printf("f1 = %p, f2 = %p\n", (void *)&f1, (void *)&f2);
	return 0;
}
