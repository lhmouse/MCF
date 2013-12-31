#include "StdMCF.hpp"
#include "Core/Format.hpp"
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <windows.h>

unsigned int MCFMain(){
	MCF::ANSIString s;
	MCF::Format(s, "%0; + %1; = %2;", 4, 5, 9);
	std::puts(s.GetCStr());
	return 0;
}
