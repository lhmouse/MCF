#include <MCF/StdMCF.hpp>

extern "C" unsigned _MCFCRT_Main(){
	try {
		throw 12345;
	} catch(int e){
		__builtin_printf("caught exception: e = %d\n", e);
	}
	return 0;
}
