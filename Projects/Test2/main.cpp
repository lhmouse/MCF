#include <MCF/StdMCF.hpp>
#include <MCF/Core/Exception.hpp>
#include <MCF/Core/StreamBuffer.hpp>
#include <iostream>
using namespace std;
using namespace MCF;

unsigned int MCFMain()
try {
	StreamBuffer buf, buf2;
	static const char data[] = "hello world! ";
	copy(begin(data), end(data) - 1, buf.GetWriteIterator());
	buf.Append(buf);
	buf2 = move(buf);
	copy(buf2.GetReadIterator(), buf2.GetReadEnd(), ostream_iterator<char>(cout));

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
