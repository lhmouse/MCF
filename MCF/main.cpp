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
	MCF::ANSIString str;

	for(int i = 0; i < 90; ++i){
		str.Unshift('b', 1);
	}
	for(int i = 0; i < 90; ++i){
		str.Append('a', 1);
	}
	std::puts((const char *)str);

	return 0;
}
