#include <MCF/StdMCF.hpp>
#include <MCF/Core/Exception.hpp>
#include <MCF/Thread/Thread.hpp>
using namespace std;
using namespace MCF;

unsigned int MCFMain()
try {
	std::vector<std::shared_ptr<Thread>> v;
	v.resize(200);
	for(auto &p : v){
		p = Thread::Create(
			[]{
				::Sleep(1000);
				puts("meow"_AS.GetCStr());
			}
		);
	}
	for(auto &p : v){
		p->Join();
	}

	return 0;
} catch(exception &e){
	printf("exception '%s'\n", e.what());
	auto *const p = dynamic_cast<const Exception *>(&e);
	if(p){
		printf("  err  = %lu\n", p->m_ulErrorCode);
		printf("  desc = %s\n", AnsiString(GetWin32ErrorDesc(p->m_ulErrorCode)).GetCStr());
		printf("  func = %s\n", p->m_pszFunction);
		printf("  line = %lu\n", p->m_ulLine);
		printf("  msg  = %s\n", AnsiString(p->m_wcsMessage).GetCStr());
	}
	return 0;
}
