#include <MCF/StdMCF.hpp>
#include <MCF/Core/DynamicLinkLibrary.hpp>
#include <cstdio>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	using namespace MCF;

	const DynamicLinkLibrary dll(L"MCFCRT-9.DLL"_wsv);
	const auto f = dll.RequireProcAddress<double (*)(double)>("trunc"_nsv);

	for(int i = -100; i <= 100; ++i){
		const auto d = static_cast<double>(i) / 10;
		std::printf("f(%f) = %.16f\n", d, f(d));
	}

	return 0;
}
