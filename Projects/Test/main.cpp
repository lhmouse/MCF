#include <MCF/StdMCF.hpp>
#include <MCF/Core/Formatters.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	int i = 123;
	double d = 456.789;
	char temp[64];
	PRINTER(str, dec, str, hex)(temp, "i = ", i, ", d = ", d)[0] = 0;
	std::puts(temp);
	return 0;
}
