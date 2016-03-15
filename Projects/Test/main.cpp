#include <MCF/StdMCF.hpp>
#include <MCF/Function/BlackHole.hpp>

extern "C" unsigned MCFCRT_Main(){
	auto a = (MCF::BlackHole() = 1);
	return 0;
}
