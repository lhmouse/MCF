#include <MCF/StdMCF.hpp>
#include <MCF/StreamFilters/Lzma.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	StreamBuffer buf;

	buf.Put(" Hello world! Hello world! Hello world!");
	LzmaEncoder().FilterInPlace(buf);
	LzmaDecoder().FilterInPlace(buf);

	buf.Traverse([](auto p, auto cb){
		for(unsigned i = 0; i < cb; ++i){ std::putchar(((const char *)p)[i]); }
	});

	return 0;
}
