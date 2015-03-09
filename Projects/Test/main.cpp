#include <MCF/StdMCF.hpp>
#include <MCF/Core/Print.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
/*	using namespace MCF::Printers;

	constexpr auto v = 0x1234abcd;

	char str[64];
	Print<X>(str, v)[0] = 0;
	std::printf("result = %s\n", str);*/

	auto ptr = std::malloc(10);
	ptr = std::realloc(ptr, 40);
	ptr = std::realloc(ptr, 20);
	std::free(ptr);

	return 0;
}
