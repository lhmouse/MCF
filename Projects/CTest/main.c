#include <MCFCRT/ext/alloca.h>

unsigned int _MCFCRT_Main(void){
	__debugbreak();
	_MCFCRT_ALLOCA(100);
	__debugbreak();
	_MCFCRT_ALLOCA(1000);
	__debugbreak();
	_MCFCRT_ALLOCA(10000);
	__debugbreak();
	_MCFCRT_ALLOCA(100000);
	return 0;
}
