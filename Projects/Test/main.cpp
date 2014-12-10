#include <MCF/StdMCF.hpp>
#include <MCF/Core/StreamBuffer.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	{
		StreamBuffer buf;
		for(unsigned i = 0; i < 100000; ++i){
			buf.Put(i);
		}
	}
	{
		StreamBuffer buf;
		for(unsigned i = 0; i < 100000; ++i){
			buf.Unget(i);
		}
	}
	return 0;
}
