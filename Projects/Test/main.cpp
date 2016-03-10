#include <MCF/StdMCF.hpp>
#include <MCF/Core/Rcnts.hpp>

using namespace MCF;

extern "C" unsigned MCFCRT_Main(){
	auto s1 = Rcntu8s::Copy("hello");
	auto s2 = Rcntu8s::View("hello!");
	std::printf("gt? %d\n", s1 >= s2);

	return 0;
}
