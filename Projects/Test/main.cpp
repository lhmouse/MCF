#include <MCF/StdMCF.hpp>
#include <MCF/Thread/CallOnce.hpp>
using namespace MCF;

OnceFlag fl;

extern "C" unsigned int MCFMain() noexcept {
	CallOnce(fl, []{ std::puts("meow!"); });
	CallOnce(fl, []{ std::puts("meow!"); });
	return 0;
}
