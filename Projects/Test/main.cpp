#include <MCF/Core/DynamicLinkLibrary.hpp>
#include <cstdio>

using namespace MCF;

extern "C" unsigned MCFMain(){
	DynamicLinkLibrary dll(L"msvcrt.dll"_wsv);
	auto pfn = dll.GetProcAddress<void (*)(void)>(5);
	std::printf("pfn = %p\n", (void *)pfn);
	return 0;
}
