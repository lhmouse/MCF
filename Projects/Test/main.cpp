#include <MCF/StdMCF.hpp>
#include <MCF/Core/DynamicLinkLibrary.hpp>
#include <cstdio>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	using namespace MCF;

	const DynamicLinkLibrary dll(L"MCFCRT-9.DLL"_wsv);
	const auto f = dll.RequireProcAddress<double (*)(double, double)>("fmod"_nsv);

	for(int i = -100; i <= 100; ++i){
		const auto d = static_cast<float>(i / 10.0);
		std::printf("f(%f) = %.16f\n", (double)d, (double)f(d, 1.5));
	}

	return 0;
}
