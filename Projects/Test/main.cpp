#include <MCF/StdMCF.hpp>
#include <MCF/Core/DynamicLinkLibrary.hpp>
#include <cstdio>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	using namespace MCF;

	const DynamicLinkLibrary dll(L"MCFCRT-9.DLL"_wsv);
	const auto my_fmal = dll.RequireProcAddress<long double (*)(long double, long double, long double)>("fmal"_nsv);

	const long double x = +(1 + 0x3p-40l);
	const long double y = +(1 + 0x5p-40l);
	const long double z = -(1 + 0x8p-40l);
	__builtin_printf("my_fma(x,y,z) = %.30a\n", (double)my_fmal(x, y, z));

	return 0;
}
