#include <MCF/StdMCF.hpp>

extern "C" unsigned _MCFCRT_Main(){
	delete new int;

	return 0;
}
