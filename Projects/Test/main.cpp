#include <MCF/StdMCF.hpp>
#include <MCF/Core/StreamBuffer.hpp>
#include <MCF/StreamFilters/ZlibFilters.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	StreamBuffer buf;
	for(int i = 0; i < 1000; ++i){
		buf.Put("meow");
	}
	ZlibEncoder().FilterInPlace(buf);
	std::printf("%zu\n", buf.GetSize());
	return 0;
}
