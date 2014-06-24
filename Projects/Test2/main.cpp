#include <MCF/StdMCF.hpp>
#include <MCFCRT/exe/exe_decl.h>
#include <MCFCRT/env/hooks.h>
#include <cstdlib>
using namespace MCF;

void MCF_OnException(void *p, const std::type_info &ti, const void *ret) noexcept {
	std::printf("  exception '%s' thrown at %p\n", ti.name(), ret);
}

extern "C" void MCF_OnHeapAlloc(void *p, std::size_t cb, const void *ret) noexcept {
	std::printf("  heap alloc,   p = %p, cb = %5zu, ret = %p\n", p, cb, ret);
}
extern "C" void MCF_OnHeapDealloc(void *p, std::size_t cb, const void *ret) noexcept {
	std::printf("  heap dealloc, p = %p, cb = %5zu, ret = %p\n", p, cb, ret);
}

void foo(){
	std::printf("will throw exception\n");
	throw 123;
}

extern "C" unsigned int MCFMain() noexcept {
	try {
		foo();
	} catch(int e){
		std::printf("exception caught, e = %d\n", e);
	}
	return 0;
}
