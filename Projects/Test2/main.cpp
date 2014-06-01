#include <MCF/StdMCF.hpp>
#include <MCF/Core/Exception.hpp>
using namespace std;
using namespace MCF;

unsigned int MCFMain()
try {
	char str[] = "0123456789";
	char str2[sizeof(str)] = { };
	ReverseMoveBackward(end(str2) - 1, begin(str), end(str) - 1);
	puts(str2);
	return 0;
} catch(exception &e){
	printf("exception '%s'\n", e.what());
	auto *const p = dynamic_cast<const Exception *>(&e);
	if(p){
		printf("  err  = %lu\n", p->m_ulErrorCode);
		printf("  desc = %s\n", AnsiString(GetWin32ErrorDesc(p->m_ulErrorCode)).GetCStr());
		printf("  func = %s\n", p->m_pszFunction);
		printf("  line = %lu\n", p->m_ulLine);
		printf("  msg  = %s\n", AnsiString(WideString(p->m_wcsMessage)).GetCStr());
	}
	return 0;
}
