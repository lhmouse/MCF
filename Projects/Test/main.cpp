#include <MCF/StdMCF.hpp>
#include <MCF/Function/WhiteHole.hpp>

extern "C" unsigned MCFCRT_Main(){
	auto wh = MCF::WhiteHole();
	int i = wh;
	double d = wh;
	std::printf("i = %d, d = %f\n", i, d);
	return 0;
}
