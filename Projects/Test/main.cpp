#include <MCF/StdMCF.hpp>
#include <MCF/Core/StreamBuffer.hpp>
#include <cstdio>

using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	StreamBuffer buf;
	for(std::size_t i = 0; i < 10000; ++i){
		buf.Put(i);
	}
	buf.Clear();
	for(std::size_t i = 0; i < 10000; ++i){
		buf.Put(i);
	}
	return 0;
}
