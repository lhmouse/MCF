#include <MCF/StdMCF.hpp>
#include <cmath>

volatile auto pf1 = (double (*)(double))&std::cos;
volatile auto pf2 = (double (*)(double))::GetProcAddress(::LoadLibraryA("msvcrt.dll"), "cos");

extern "C" unsigned MCFCRT_Main(){
	const auto r = 0x1p110;

	std::printf("val = %f, msvc = %f, const = %f\n", (*pf1)(r), (*pf2)(r), __builtin_cos(r));

	return 0;
}
