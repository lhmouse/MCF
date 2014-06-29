#include <MCF/StdMCF.hpp>
#include <MCF/Core/StreamBuffer.hpp>
#include <iostream>
using namespace MCF;
/*
extern "C" void MCF_OnHeapAlloc(void *p, std::size_t cb, const void *ret) noexcept {
	std::printf("  heap alloc,   p = %p, cb = %5zu, ret = %p\n", p, cb, ret);
}
extern "C" void MCF_OnHeapDealloc(void *p, const void *ret) noexcept {
	std::printf("  heap dealloc, p = %p, ret = %p\n", p, ret);
}
*/
extern "C" unsigned int MCFMain() noexcept {
	for(std::size_t i = 0; i < 80; ++i){
		static const unsigned char data[] = "0123456789abcdefghijklmnopqrstuvexyzABCDEFGHIJKLMNOPQRSTUVEXYZ";
		StreamBuffer buf, buf2;
		buf.Insert(data, sizeof(data));
		buf.CutOut(buf2, i);

		Copy(std::ostream_iterator<unsigned char>(std::cout), buf2.GetReadIterator(), buf2.GetReadEnd());
		std::cout <<'$' <<std::endl;
		Copy(std::ostream_iterator<unsigned char>(std::cout), buf.GetReadIterator(), buf.GetReadEnd());
		std::cout <<'$' <<std::endl;
	}
	return 0;
}
