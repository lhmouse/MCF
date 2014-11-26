#include <MCF/StdMCF.hpp>
#include <MCF/Core/StreamBuffer.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	StreamBuffer buf;
	buf.Splice(StreamBuffer("a"));
	buf.Splice(StreamBuffer("b"));
	buf.Splice(StreamBuffer("c"));
	StreamBuffer(buf).Swap(buf);
	buf.Traverse(
		[](auto p, auto cb){
			std::fwrite(p, cb, 1, stdout);
			std::putchar('\n');
		}
	);
	return 0;
}
