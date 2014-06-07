#include <MCF/StdMCF.hpp>
#include <MCF/Core/Exception.hpp>
#include <MCF/Core/MessageDriver.hpp>
using namespace std;
using namespace MCF;

unsigned int MCFMain()
try {
	auto p1 = Message::RegisterHandler("meow"_u8so, [](auto &){ puts("handler1"); return false; });
	auto p2 = Message::RegisterHandler("meow"_u8so, [](auto &){ puts("handler2"); return false; });

	Message x("meow"_u8s);
	x.Dispatch();
	p1.reset();
	puts("----- reset p1");
	x.Dispatch();

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
