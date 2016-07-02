#include <MCF/StdMCF.hpp>

char a[4096] = { };
char b[4096] = { };
void *(*p)(void *, const void *, std::size_t) = &std::memcpy;

extern "C" unsigned _MCFCRT_Main(void){
	(*p)(a, b, sizeof(a));
	return 0;
}
