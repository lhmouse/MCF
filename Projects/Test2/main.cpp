#include <MCF/StdMCF.hpp>
#include <MCF/Core/Exception.hpp>
#include <MCF/Core/StreamBuffer.hpp>
#include <MCF/Thread/CriticalSection.hpp>
#include <iostream>
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

	buf.Traverse(
		[](auto p, auto cb){
			while(cb--){
				putchar(*(p++));
			}
			putchar('\n');
		}
	);

	copy(buf.GetReadIterator(), buf.GetReadEnd(), ostream_iterator<char>(cout));
	cout <<endl;

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
