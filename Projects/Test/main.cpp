#include <MCF/Core/LastError.hpp>
#include <MCF/Core/DynamicLinkLibrary.hpp>
#include <winternl.h>
#include <cstdio>
#include <typeinfo>

using namespace MCF;

extern "C" unsigned MCFMain(){
	DynamicLinkLibrary d(L"msvcrt.dll"_wsv);
	const auto f = d.RequireProcAddress<int (*)(const char *)>("puts"_nsv);
	(*f)("hello world!");
	return 0;
}
