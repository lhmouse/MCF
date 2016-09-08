#include <MCF/StdMCF.hpp>
#include <MCF/Core/DynamicLinkLibrary.hpp>
#include <cstdio>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	using namespace MCF;

	const DynamicLinkLibrary dll(L"MCFCRT-9.DLL"_wsv);
	const auto my_fmal = dll.RequireProcAddress<long double (*)(long double, long double, long double)>("fmal"_nsv);

	volatile long double x =  1.00000000002l;
	volatile long double y =  3.00000000004l;
	volatile long double z = -3.00000000010l;
	std::printf("my_fmal(x,y,z) = %.30Le\n", my_fmal(x, y, z));

	return 0;
}
