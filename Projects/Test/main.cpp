#include <MCF/StdMCF.hpp>
#include <MCF/Core/DynamicLinkLibrary.hpp>
#include <cstdio>

using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	DynamicLinkLibrary dll(L"msvcr100.dll");
	auto pfn = dll.GetProcAddress<int __cdecl(const char *, ...)>("printf");
	if(!pfn){
		std::puts("failed to get address of printf()");
	} else {
		(*pfn)("hello world! i = %d\n", 123);
	}
	return 0;
}
