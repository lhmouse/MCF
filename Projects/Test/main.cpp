#include <MCF/StdMCF.hpp>
#include <MCF/Core/TupleManipulators.hpp>
#include <cstdio>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	MCF::ReverseSqueeze([](auto ...i){ (std::printf("%d ", (int)i), ...); }, std::make_tuple(1,2,3,4,5));
	return 0;
}
