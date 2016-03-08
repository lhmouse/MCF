#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>

using namespace MCF;

extern "C" unsigned MCFCRT_Main(){
	try {
		Utf32String s;
		s.Resize(0x50000000);
	} catch(std::exception &e){
		std::printf("exception: what = %s\n", e.what());
	}

	return 0;
}
