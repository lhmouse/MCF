#include <MCF/StdMCF.hpp>
#include <MCF/Core/Exception.hpp>
#include <MCF/Core/StreamBuffer.hpp>
#include <MCF/Thread/CriticalSection.hpp>
using namespace std;
using namespace MCF;

unsigned int MCFMain()
try {
	auto cs = CriticalSection::Create();
	cs->GetLock();

	StreamBuffer buf;
	buf.Insert("a", 1);
	buf.Insert("bc", 2);
	buf.Insert("def", 3);
	buf.Insert("ghij", 4);
	buf.Insert("klmno", 5);
	char temp[16];
	buf.Extract(temp, 15);
	temp[15] = 0;
	puts(temp);
	return 0;
} catch(exception &e){
	printf("exception '%s'\n", e.what());
	auto *const p = dynamic_cast<const Exception *>(&e);
	if(p){
		printf("  err  = %lu\n", p->ulErrorCode);
		printf("  desc = %s\n", AnsiString(GetWin32ErrorDesc(p->ulErrorCode)).GetCStr());
		printf("  func = %s\n", p->pszFunction);
		printf("  line = %lu\n", p->ulLine);
		printf("  msg  = %s\n", AnsiString(WideString(p->pwszMessage)).GetCStr());
	}
	return 0;
}
