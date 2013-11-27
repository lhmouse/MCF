//#include "Core/Format.hpp"
#include "MCFCRT/MCFCRT.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>

extern "C" int MCFMain(std::size_t, const wchar_t *const *){
	int *const p = new int[5];
	p[5] = 0;
	delete[] p;

	return 0;
}
