#include <MCF/StdMCF.hpp>
#include <MCF/Core/Optional.hpp>
#include <cstdio>

template class MCF::Optional<int>;

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	MCF::Optional<int> o;
	o = 1;
	o = 2;
	return 0;
}
