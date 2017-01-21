#include <MCF/StdMCF.hpp>
#include <MCFCRT/ext/alloca.h>
#include <cstdio>

__attribute__((__noinline__))
void foo(void *ptr, char &a){
	char b;
	std::printf("ptr = %p, d = %tx\n", ptr, &a - &b);
}

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	char a;
	auto ptr = _MCFCRT_ALLOCA(2064384);
	foo(ptr, a);
	return 0;
}
