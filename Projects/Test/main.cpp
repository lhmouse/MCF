#include <MCF/StdMCF.hpp>
#include <MCF/Core/DynamicLinkLibrary.hpp>
#include <cstdio>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	using namespace MCF;

	const DynamicLinkLibrary dll(L"MCFCRT-9.DLL"_wsv);
#define Z(e_)   #e_, (double)(e_)

	const auto ff = dll.RequireProcAddress<float (*)(float, float)>("fdimf"_nsv);
	std::printf("ff = %p, %s = %f, %s = %f\n", (void *)ff, Z(ff(1.2f, 2.1f)), Z(ff(-1.2f, -2.1f)));

	const auto fd = dll.RequireProcAddress<double (*)(double, double)>("fdim"_nsv);
	std::printf("fd = %p, %s = %f, %s = %f\n", (void *)fd, Z(fd(1.2 , 2.1 )), Z(fd(-1.2 , -2.1 )));

	const auto fl = dll.RequireProcAddress<long double (*)(long double, long double)>("fdiml"_nsv);
	std::printf("fl = %p, %s = %f, %s = %f\n", (void *)fl, Z(fl(1.2l, 2.1l)), Z(fl(-1.2l, -2.1l)));

	return 0;
}
