#include <MCF/StdMCF.hpp>
#include <MCF/Core/DynamicLinkLibrary.hpp>
#include <cstdio>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	using namespace MCF;

	const DynamicLinkLibrary dll(L"MCFCRT-9.DLL"_wsv);
	const auto f = dll.RequireProcAddress<float (*)(float, float)>("fdimf"_nsv);

	for(int i = -10; i <= 10; ++i){
		const auto d = static_cast<float>(i);
		std::printf("f(%f) = %.16f\n", (double)d, (double)f(d, 5));
	}

	return 0;
}
