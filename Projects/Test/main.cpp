#include <MCF/StdMCF.hpp>
#include <MCF/Core/Print.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	char temp[64];
	constexpr auto d = 0x1234abcd;
	Print<Printers::x>(temp, d)[0] = 0;
	std::printf("result = %s\n", temp);
	return 0;
}
