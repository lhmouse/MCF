#include <MCF/StdMCF.hpp>
#include <MCF/Core/Time.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	std::printf("%016llX\n", (unsigned long long)GetUtcTime());
	std::printf("%016llX\n", (unsigned long long)GetLocalTime());
	return 0;
}
