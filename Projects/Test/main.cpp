#include <MCF/StdMCF.hpp>
#include <MCF/Core/SharedString.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	auto p = CreateSharedString("hello world!", 12);
	return 0;
}
