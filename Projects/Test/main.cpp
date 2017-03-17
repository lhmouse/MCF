#include <MCF/Function/Function.hpp>
#include <MCF/Function/FunctionView.hpp>
#include <cstdio>

using namespace MCF;

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	auto fp = MakeUniqueFunction<long (int)>([](int a){ return std::printf("a = %d\n", a); });
	auto fv = FunctionView<void (int)>(fp);
	(*fp)(12345);
	fv(67890);
	return 0;
}
