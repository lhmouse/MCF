#include <MCF/StdMCF.hpp>
#include <MCF/Utilities/CopyMoveFill.hpp>

using namespace MCF;

extern "C" unsigned MCFCRT_Main(){
	auto src = {'0','1','2','3','4'};
	char dst[64] = { };
	CopyIntoBackward(dst, dst + 5, src.end());
	std::puts(dst);
	return 0;
}
