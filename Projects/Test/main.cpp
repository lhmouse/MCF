#include <MCF/StdMCF.hpp>
#include <cstdio>

struct foo {
	const char *const ctx;

	explicit foo(const char *s)
		: ctx(s)
	{
		std::printf("ctor: %s\n", ctx);
	}
	~foo(){
		std::printf("dtor: %s\n", ctx);
	}
};

extern "C" unsigned _MCFCRT_Main(){
	static foo f1("static");
	std::printf("f1 = %p\n", (void *)&f1);
	return 0;
}
