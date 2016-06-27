#include <MCF/StdMCF.hpp>
#include <MCFCRT/env/bail.h>

extern "C" unsigned _MCFCRT_Main(void){
	try {
		throw 12345;
	} catch(int e){
		__builtin_printf("exception caught: e = %d\n", e);
	}
	return 0;
}
