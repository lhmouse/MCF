#include <MCF/StdMCF.hpp>
#include <MCFCRT/env/bail.h>

struct foo {
	foo(){
		__builtin_puts("constructed!");
	}
	~foo(){
		__builtin_puts("destructed!");
	}
};

extern "C" unsigned _MCFCRT_Main(void){
	try {
		foo f;
		throw 12345;
	} catch(int e){
		__builtin_printf("exception caught: e = %d\n", e);
	}
		throw 12345;
	return 0;
}
