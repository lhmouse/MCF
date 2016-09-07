#include <MCF/StdMCF.hpp>
#include <MCF/Core/DynamicLinkLibrary.hpp>
#include <cstdio>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	using namespace MCF;

	const DynamicLinkLibrary dll(L"MCFCRT-9.DLL"_wsv);
//	const auto my_sinl = dll.RequireProcAddress<long double (*)(long double)>("sinl"_nsv);
	const auto my_cosl = dll.RequireProcAddress<long double (*)(long double)>("cosl"_nsv);
/*
	const auto theta = 1.0e1000l;
	const auto r1 = my_sinl(theta);
	const auto r2 = 2 * my_sinl(theta / 2) * my_cosl(theta / 2);
	std::printf("r1 = %.16Lf\nr2 = %.16Lf\n", r1, r2);
*/
	for(int i = -24; i < 24; ++i){
		const auto d = i / 6.0l * 3.1415926535897932384626433832795l;
		std::printf("cos(%20.16Lf) = %20.16Lf\n", d, my_cosl(d));
	}

	return 0;
}
