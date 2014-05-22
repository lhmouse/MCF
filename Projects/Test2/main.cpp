#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Core/Exception.hpp>
#include <MCF/Serialization/Serdes.hpp>
using namespace std;
using namespace MCF;

struct foo {
	int i[2];
};

SERDES_TABLE_BEGIN(foo)
	SERDES_MEMBER(i)
SERDES_TABLE_END

struct bar : foo {
	double d;
	char c;
};

SERDES_TABLE_BEGIN(bar)
	SERDES_MEMBER(d)
	SERDES_MEMBER(c)
	SERDES_BASE(foo)
SERDES_TABLE_END

unsigned int MCFMain()
try {
	DataBuffer dbuf;

	bar b1, b2;
	b1.i[0] = 123;
	b1.i[1] = 456;
	b1.d = 7.0;
	b1.c = '8';

	Serialize(dbuf, b1);
	printf("serialized: %zu bytes\n", dbuf.GetSize());
	Deserialize(b2, dbuf);

	printf("[%d %d] %f %c\n", b2.i[0], b2.i[1], b2.d, b2.c);

	return 0;
} catch(exception &e){
	printf("exception '%s'\n", e.what());
	auto *const p = dynamic_cast<const Exception *>(&e);
	if(p){
		printf("  err  = %lu\n", p->ulErrorCode);
		printf("  desc = %s\n", AnsiString(GetWin32ErrorDesc(p->ulErrorCode)).GetCStr());
		printf("  func = %s\n", p->pszFunction);
		printf("  msg  = %s\n", AnsiString(WideString(p->pwszMessage)).GetCStr());
	}
	return 0;
}
