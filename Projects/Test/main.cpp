#include <MCF/StdMCF.hpp>
#include <MCF/Core/SharedString.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	auto p = CreateSharedString(L"hello world!"_wso, 6);
	std::printf("-- cap = %u\n", (unsigned)GetSharedStringCapacity(p));
	std::printf("-- len = %u\n", (unsigned)GetSharedStringLength(p));
	return 0;
}
