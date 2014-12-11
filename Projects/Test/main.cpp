#include <MCF/StdMCF.hpp>
#include <MCF/Utilities/Utilities.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	DEFER([]{ std::puts("meow 1"); });
	DEFER([]{ std::puts("meow 2"); });
	DEFER([]{ std::puts("meow 3"); });
	return 0;
}
