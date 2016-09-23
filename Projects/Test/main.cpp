#include <MCF/StdMCF.hpp>
#include <MCF/Core/Defer.hpp>
#include <cstdio>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	try {
		auto dg = MCF::Defer             ([]{ std::puts("deferred!"); });
		auto dn = MCF::DeferOnNormalExit ([]{ std::puts("deferred on normal exit!"); });
		auto de = MCF::DeferOnException  ([]{ std::puts("deferred on exception!"); });
		//throw 1;
	} catch(int e){
		std::printf("exception caught: e = %d\n", e);
	}
	return 0;
}
