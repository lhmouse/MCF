#include <MCF/StdMCF.hpp>
#include <MCF/Core/Random.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Core/Exception.hpp>
#include <MCF/Serialization/Serdes.hpp>
using namespace std;
using namespace MCF;

struct foo {
	int i[2];
	double d;
	char c;
};

unsigned int MCFMain()
try {
	Random rng;
	DataBuffer dbuf;

	map<int, double> l1, l2;
	for(int i = 0; i < 10; ++i){
		l1.insert(make_pair(i, i * i));
	}

	for(auto i : l1){
		printf("%d %f; ", i.first, i.second);
	}
	putchar('\n');

	Serialize(dbuf, l1);
	printf("serialized: %zu bytes\n", dbuf.GetSize());
	Deserialize(l2, dbuf);

	for(auto i : l2){
		printf("%d %f; ", i.first, i.second);
	}
	putchar('\n');

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
