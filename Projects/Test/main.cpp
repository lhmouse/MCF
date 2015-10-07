#include <MCF/Core/LastError.hpp>
#include <MCF/Core/DynamicLinkLibrary.hpp>
#include <cstdio>

using namespace MCF;

extern "C" unsigned MCFMain(){
	DynamicLinkLibrary d(L"ntdll.dll");
	return 0;
}
