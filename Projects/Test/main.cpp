#include <MCF/StdMCF.hpp>
#include <MCF/Core/StreamBuffer.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	StreamBuffer buf;
	std::puts("------ 1");
	buf.Put('a');
	std::puts("------ 2");
	buf.Put('b');
	std::puts("------ 3");
	buf.Get();
	std::puts("------ 4");
	buf.Get();
	std::puts("------ 5");
	buf.Put('a');
	std::puts("------ 6");
	return 0;
}
