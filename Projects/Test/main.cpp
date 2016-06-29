#include <MCF/StdMCF.hpp>
#include <MCFCRT/env/bail.h>

extern "C" unsigned _MCFCRT_Main(void){
//	RaiseException(0x20474343ul, 0, 0, 0);
//	__builtin_puts("returned!");

	try {
		throw 12345;
	} catch(int e){
		__builtin_printf("exception caught: e = %d\n", e);
	}
		throw 12345;
	return 0;
}
