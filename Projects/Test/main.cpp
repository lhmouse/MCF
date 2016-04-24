#include <MCF/StdMCF.hpp>
#include <cstdio>

extern "C" unsigned _MCFCRT_Main(){
	try {
		throw 123;
	} catch(int e){
		std::printf("exception caught: e = %d\n", e);
	}
	return 0;
}
