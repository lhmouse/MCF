#include <MCF/StdMCF.hpp>
#include <MCF/Core/DynamicLinkLibrary.hpp>
#include <cstdio>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	using namespace MCF;

	const DynamicLinkLibrary dll(L"MCFCRT-9.DLL"_wsv);
	const auto my_fmal = dll.RequireProcAddress<long double (*)(long double, long double, long double)>("fmal"_nsv);

	const auto x =  1.000000002l;
	const auto y =  3.000000004l;
	const auto z = -3.000000010l;
	std::printf("my_fmal(x,y,z) = %.30Le\n", my_fmal(x,y,z));

	return 0;
}
