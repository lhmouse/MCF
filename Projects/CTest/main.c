#include <MCFCRT/env/standard_streams.h>

unsigned _MCFCRT_Main(void){
	_MCFCRT_WriteStandardOutputText(L"hello world!", 12, true);
	return 0;
}
