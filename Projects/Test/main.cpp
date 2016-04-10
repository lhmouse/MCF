#include <MCF/StdMCF.hpp>
#include <cstdlib>
#include <cstdio>

struct foo {
	const int k;

	explicit foo(int i)
		: k(i)
	{
		std::printf("ctor of %d\n", k);

		std::atexit([]{ std::printf("atexit!\n"); });
	}
	~foo(){
		std::printf("dtor of %d\n", k);
	}
} f1(1), f2(2);

extern "C" unsigned _MCFCRT_Main(){
	return 0;
}
