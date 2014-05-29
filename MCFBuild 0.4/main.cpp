// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#include "MCFBuild.hpp"
#include "Localization.hpp"
#include "../MCF/Core/Exception.hpp"
#include <exception>
#include <cstdio>
using namespace MCFBuild;

int main()
try {
	MCF_THROW(123, L"FILE_TOO_LARGE|meow"_WS);
} catch(std::exception &e){
	std::printf("exception '%s'\n", e.what());
	auto *const p = dynamic_cast<const MCF::Exception *>(&e);
	if(p){
		std::printf("  err  = %lu\n", p->m_ulErrorCode);
		std::printf("  desc = %s\n", MCF::AnsiString(MCF::GetWin32ErrorDesc(p->m_ulErrorCode)).GetCStr());
		std::printf("  func = %s\n", p->m_pszFunction);
		std::printf("  line = %lu\n", p->m_ulLine);
		std::printf("  msg  = %s\n", MCF::AnsiString(FormatString(p->m_pwszMessage.get())).GetCStr());
	}
	return 0;
}
