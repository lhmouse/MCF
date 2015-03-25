#include <MCF/StdMCF.hpp>
#include <MCFCRT/env/hooks.h>
#include <vector>
using namespace MCF;

__attribute__((__constructor__))
static void init_hooks(){
	::__MCF_OnHeapAlloc = [](auto p, auto size, auto ret){
		std::printf("heap alloc:   p = %p, size = %zu, ret = %p\n", p, size, ret);
	};
	::__MCF_OnHeapRealloc = [](auto p, auto old, auto size, auto ret){
		std::printf("heap realloc: p = %p, old = %p, size = %zu, ret = %p\n", p, old, size, ret);
	};
	::__MCF_OnHeapDealloc = [](auto p, auto ret){
		std::printf("heap dealloc: p = %p, ret = %p\n", p, ret);
	};
}

extern "C" unsigned int MCFMain() noexcept {
	std::vector<int> v;
	for(int i = 0; i < 10000; ++i){
		v.push_back(i);
	}
	return 0;
}
