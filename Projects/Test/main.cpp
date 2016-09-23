#include <MCF/StdMCF.hpp>
#include <MCF/Core/BlackWhiteHole.hpp>
#include <cstdio>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	MCF::BlackHole bh;
	MCF::WhiteHole wh;
	int a = wh;
	std::printf("a = %d\n", a);
	bh = a;
	return 0;
}
