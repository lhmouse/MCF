#include <MCF/StdMCF.hpp>
#include <MCF/Utilities/CountLeadingTrailingZeroes.hpp>
#include <cstdio>

using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	constexpr std::uint32_t v = 0x12345;
	std::int32_t a[CountLeadingZeroes(v)];
	std::printf("sizeof(a) = %zu\n", sizeof(a));
	return 0;
}
