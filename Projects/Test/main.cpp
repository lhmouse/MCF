#include <MCFCRT/MCFCRT.h>

extern "C" unsigned _MCFCRT_Main() noexcept {
	for(int i = 0; i < 10001; ++i){
		::_MCFCRT_malloc(10);
	}
	return 0;
}
