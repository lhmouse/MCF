#include <MCF/StdMCF.hpp>
#include <MCF/Core/DynamicLinkLibrary.hpp>
#include <cmath>
#include <cstdio>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	using namespace MCF;

	const DynamicLinkLibrary my(L"MCFCRT-9.DLL"_wsv);
	const auto my_sinl = my.RequireProcAddress<long double (*)(long double)>("sinl"_nsv);

	volatile auto one = 1.0l;
	auto theta = atanl(one) * 4;
	std::printf("sinl   (theta) = %.16Le\n", sinl   (theta));
	std::printf("my_sinl(theta) = %.16Le\n", my_sinl(theta));

	return 0;
}
