#include <MCF/StdMCF.hpp>
#include <MCF/Core/Invoke.hpp>

void foo(){
	auto k = [](int a){ return __builtin_printf("a = %d\n", a); };
	return MCF::DesignatedInvoke<void>(k, 1);
}

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	f();
	return 0;
}
