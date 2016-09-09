#include <MCF/StdMCF.hpp>
#include <MCF/Core/DynamicLinkLibrary.hpp>
#include <cstdio>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	using namespace MCF;

	const DynamicLinkLibrary dll(L"MCFCRT-9.DLL"_wsv);
	const auto my_fma = dll.RequireProcAddress<double (*)(double, double, double)>("fma"_nsv);

	const double x =   1 + 0x3p-40;
	const double y =   1 + 0x5p-40;
	const double z = -(1 + 0x8p-40);
	__builtin_printf("my_fma(x,y,z) = %.30a\n", my_fma(x, y, z));

	return 0;
}
