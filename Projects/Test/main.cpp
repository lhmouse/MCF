#include <MCF/StdMCF.hpp>
#include <MCF/Function/BlackHole.hpp>

using namespace MCF;

extern "C" unsigned MCFCRT_Main(){
	BlackHole b;
	b = 1;
	b = 2;
	return 0;
}
