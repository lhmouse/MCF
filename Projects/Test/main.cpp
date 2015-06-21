#include <MCF/StdMCF.hpp>
#include <MCF/Core/DynamicLinkLibrary.hpp>
#include <MCF/Core/Exception.hpp>
#include <MCF/Core/LastError.hpp>
#include <cstdio>

using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	DynamicLinkLibrary dll(L"msvcr100.dll");
	try {
		auto pfn = dll.RequireProcAddress<int __cdecl(const char *, ...)>("printf_not_exist");
	} catch(SystemError &e){
		std::printf("SystemError caught: code = %lu, desc = %s\n", e.GetCode(), AnsiString(GetWin32ErrorDescription(e.GetCode())).GetStr());
	}
	return 0;
}
