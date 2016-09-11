#include <MCF/StdMCF.hpp>
#include <MCF/Containers/FlatMap.hpp>

extern MCF::FlatMap<int, class Incomplete> map;

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	return 0;
}
