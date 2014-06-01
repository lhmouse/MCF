// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#include "MCFBuild.hpp"
#include "Localization.hpp"
#include "FileSystem.hpp"
#include "../MCF/Core/Exception.hpp"
#include <exception>
#include <cstdio>
using namespace MCFBuild;

extern "C" unsigned MCFMain()
try {
	Sha256 sha;
	bool ret = GetFileContentsAndSha256(nullptr, sha, L"E:\\Desktop\\HttpClient.cpp"_ws);
	std::printf("%d\n", ret);
	for(auto by : sha){
		std::printf("%02hhX", by);
	}
	std::putchar('\n');
	return 0;
} catch(std::exception &e){
	std::printf("exception '%s'\n", e.what());
	auto *const p = dynamic_cast<const MCF::Exception *>(&e);
	if(p){
		std::printf("  err  = %lu\n", p->m_ulErrorCode);
		std::printf("  desc = %s\n", MCF::AnsiString(MCF::GetWin32ErrorDesc(p->m_ulErrorCode)).GetCStr());
		std::printf("  func = %s\n", p->m_pszFunction);
		std::printf("  line = %lu\n", p->m_ulLine);
		std::printf("  msg  = %s\n", MCF::AnsiString(FormatString(p->m_wcsMessage)).GetCStr());
	}
	return 0;
}
