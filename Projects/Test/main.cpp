#include <MCF/StdMCF.hpp>
#include <MCF/Core/DynamicLinkLibrary.hpp>
#include <cstdio>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	using namespace MCF;

	const DynamicLinkLibrary dll(L"MCFCRT-9.DLL"_wsv);
	const auto my_fmal = dll.RequireProcAddress<long double (*)(long double, long double, long double)>("fmal"_nsv);

	const long double x =  1.234567890123456789l;
	const long double y =  9.876543210987654321l;
	const long double z = -12.193263113702179522374638011113l;
	std::printf("my_fmal(x,y,z) = %.30Le\n", my_fmal(x, y, z));

	return 0;
}
