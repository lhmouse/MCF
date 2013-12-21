#include "StdMCF.hpp"
//#include "Core/Format.hpp"
#include "MCFCRT/MCFCRT.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <vector>
#include <string>
#include <windows.h>

unsigned int MCFMain(){
	wchar_t buf1[100];
	wchar_t buf2[100] = L"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ/*-+%";
	std::wcscpy(buf1, buf2);
	std::fputws(buf1, stdout);
	::__MCF_Bail(buf1);
	return 0;
}
