#include <MCF/StdMCF.hpp>
#include <MCF/Core/DynamicLinkLibrary.hpp>
#include <cstdio>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	using namespace MCF;

	const DynamicLinkLibrary dll(L"MCFCRT-9.DLL"_wsv);
#define Z(e_)   #e_, (double)(e_)

	const auto ff = dll.RequireProcAddress<float (*)(float, float)>("powf"_nsv);
	std::printf("ff = %p, %s = %f, %s = %f\n", (void *)ff, Z(ff(-5.0f, -2.0f)), Z(ff(0.36f, 0.5f)));

	const auto fd = dll.RequireProcAddress<double (*)(double, double)>("pow"_nsv);
	std::printf("fd = %p, %s = %f, %s = %f\n", (void *)fd, Z(fd(-5.0 , -2.0 )), Z(fd(0.36 , 0.5 )));

	const auto fl = dll.RequireProcAddress<long double (*)(long double, long double)>("powl"_nsv);
	std::printf("fl = %p, %s = %f, %s = %f\n", (void *)fl, Z(fl(-5.0l, -2.0l)), Z(fl(0.36l, 0.5l)));

	return 0;
}
