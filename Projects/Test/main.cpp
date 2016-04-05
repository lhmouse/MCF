#include <MCF/StdMCF.hpp>
#include <cstdio>

extern "C" unsigned _MCFCRT_Main(){
	try {
		throw 12345;
	} catch(int e){
		std::printf("e = %d\n", e);
	}
	return 0;
}
