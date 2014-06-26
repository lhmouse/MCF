#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <MCFCRT/env/hooks.h>
#include <cstdlib>
using namespace MCF;

extern "C" void MCF_OnHeapAlloc(void *p, std::size_t cb, const void *ret) noexcept {
	std::printf("  heap alloc,   p = %p, cb = %5zu, ret = %p\n", p, cb, ret);
}
extern "C" void MCF_OnHeapDealloc(void *p, const void *ret) noexcept {
	std::printf("  heap dealloc, p = %p, ret = %p\n", p, ret);
}

extern "C" unsigned int MCFMain() noexcept {
std::puts("----------");
	AnsiString s	("hello world! =========");
std::puts("----------");
	s.Assign		("mmmmmeeeeeoooooowwww");
std::puts("----------");
	std::puts(s.GetCStr());

	auto p = malloc(100);
	p = realloc(p, 200);
	free(p);

	return 0;
}
