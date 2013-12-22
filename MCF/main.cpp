#include "StdMCF.hpp"
#include "MCFCRT/MCFCRT.h"
//#include "Core/Format.hpp"
#include "Core/String.hpp"
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <vector>
#include <string>
#include <windows.h>

unsigned int MCFMain(){
	MCF::ANSIString str("abcdefg");

	std::puts(str.Slice(-1, 5).GetCStr());
	std::puts("gf");

	return 0;
}
