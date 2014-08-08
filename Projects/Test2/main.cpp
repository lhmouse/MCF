#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/StreamFilters/ZLibFilters.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	StreamBuffer buf;
	const auto data = "hello world! "_nso;
	for(int i = 0; i < 100; ++i){
		buf.Insert(data.GetBegin(), data.GetSize());
	}
	ZLibEncoder().FilterInPlace(buf);
	std::printf("compressed = %zu bytes\n", buf.GetSize());
	ZLibDecoder().FilterInPlace(buf);
	auto v = buf.Squash();
	v.Push(0);
	std::printf("decompressed = %s$\n", v.GetData());
	return 0;
}
