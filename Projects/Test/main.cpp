#include <MCF/StdMCF.hpp>
#include <MCF/Core/DynamicLinkLibrary.hpp>
#include <cstdio>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	using namespace MCF;

	const DynamicLinkLibrary dll(L"MCFCRT-9.DLL"_wsv);
	const auto my_fmal = dll.RequireProcAddress<long double (*)(long double, long double, long double)>("fmal"_nsv);

	const long double x =  1.2345678901234567890l;
	const long double y =  9.8765432109876543210l;
	const long double z = -12.193263113702179522l;
	std::printf("my_fmal(x,y,z) = %.30Le\n", my_fmal(x, y, z));

	return 0;
}
