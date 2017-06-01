#include <MCFCRT/pre/module.h>
#include <cstdio>

int count;

void second(_MCFCRT_STD intptr_t param){
	std::printf("second (%d)\n", (int)param);
}
void first(_MCFCRT_STD intptr_t param){
	std::printf("first  (%d)\n", (int)param);
	_MCFCRT_AtModuleExit(&second, ++count);
}

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	for(int i = 0; i < 100; ++i){
		_MCFCRT_AtModuleExit(&first, ++count);
	}
	return 0;
}
