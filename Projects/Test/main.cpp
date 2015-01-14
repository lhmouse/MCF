#include <MCF/StdMCF.hpp>
#include <MCF/Core/StreamBuffer.hpp>
using namespace MCF;

struct foo {
	StreamBuffer b;

	foo()
		: b("hello world!")
	{
	}
};

foo f;

extern "C" unsigned int MCFMain() noexcept {
	return 0;
}
