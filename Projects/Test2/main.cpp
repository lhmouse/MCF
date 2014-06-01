#include <MCF/StdMCF.hpp>
#include <MCF/Core/Exception.hpp>
#include <MCF/Thread/Thread.hpp>
#include <MCF/Thread/MonitorPtr.hpp>
using namespace std;
using namespace MCF;

struct foo {
	int i;

	foo()
		: i(0)
	{
	}

	void inc(){
		puts("pre inc");
		::Sleep(100);
		auto tmp = i;
		::Sleep(100);
		i = tmp + 1;
		puts("post inc");
	}
};

unsigned int MCFMain()
try {
	MonitorPtr<foo> pf;

	vector<shared_ptr<Thread>> v;
	v.resize(10);
	for(auto &p : v){
		p = Thread::Create(
			[&]{
				pf->inc();
			}
		);
	}
	for(auto &p : v){
		p->Join();
	}
	printf("i = %d\n", pf->i);

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
