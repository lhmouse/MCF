#include <MCF/StdMCF.hpp>
#include <MCF/Core/Print.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	using namespace MCF::Printers;

	constexpr auto v = 0x1234abcd;

	char str[64];
	Print<X>(str, v)[0] = 0;
	std::printf("result = %s\n", str);

	return 0;
}
