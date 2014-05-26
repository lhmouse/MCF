#include <MCF/StdMCF.hpp>
#include <MCF/Core/Exception.hpp>
#include <MCF/Core/StreamBuffer.hpp>
#include <MCF/Serialization/Serdes.hpp>
using namespace std;
using namespace MCF;

unsigned int MCFMain()
try {
	auto tup1 = make_tuple(1, 2.0, '3');
	decltype(tup1) tup2;
	StreamBuffer buf;
	Serialize(buf, tup1);
	Deserialize(tup2, buf);
	printf("%d %f %c\n", get<0>(tup2), get<1>(tup2), get<2>(tup2));

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
