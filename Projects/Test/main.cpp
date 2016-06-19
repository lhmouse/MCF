#include <MCF/StdMCF.hpp>
#include <MCFCRT/env/bail.h>
#include <cstdio>

extern "C" unsigned _MCFCRT_Main(void){
	throw 12345;
}
