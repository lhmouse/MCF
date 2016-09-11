#include <MCF/StdMCF.hpp>
#include <MCF/Core/Bind.hpp>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	int k = 123;
	MCF::Bind([](const volatile int &a){ __builtin_printf("a = %d\n", a); }, k)();
	return 0;
}
