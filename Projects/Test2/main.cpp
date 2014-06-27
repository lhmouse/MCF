#include <MCF/StdMCF.hpp>
#include <MCF/Core/StreamBuffer.hpp>
#include <cstdlib>
using namespace MCF;

extern "C" void MCF_OnHeapAlloc(void *p, std::size_t cb, const void *ret) noexcept {
	std::printf("  heap alloc,   p = %p, cb = %5zu, ret = %p\n", p, cb, ret);
}
extern "C" void MCF_OnHeapDealloc(void *p, const void *ret) noexcept {
	std::printf("  heap dealloc, p = %p, ret = %p\n", p, ret);
}

extern "C" unsigned int MCFMain() noexcept {
	StreamBuffer buf;

	for(unsigned i = 0x400 - 1; i; --i){
		static const char data = 'a';
		buf.Insert(&data, sizeof(data));
	}
	for(unsigned i = 0x100000; i; --i){
		buf.Put('a');
		buf.Get();
	}

	return 0;
}
