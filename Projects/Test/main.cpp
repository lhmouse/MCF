#include <MCF/StdMCF.hpp>
#include <cstdio>

__attribute__((__nonnull__(1), __noinline__))
void foo(void *p){
	(void)(((p) ? true : false) );
	p && std::puts("always happen");
}

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	int a;
	foo(&a);
	return 0;
}
