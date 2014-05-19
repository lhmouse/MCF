#include <MCF/StdMCF.hpp>
#include <MCF/Storage/Serializer.hpp>
using namespace std;
using namespace MCF;

struct foo {
	int i[2];
	double d;
	char c;
};

void ser(SerializerDefinitions::WriteIterator &, const foo &){
	std::printf("serializing foo\n");
}
void des(foo &, SerializerDefinitions::ReadIterator &, const SerializerDefinitions::ReadIterator &){
	std::printf("unserializing foo\n");
}

SER_TABLE_TABLE_BEGIN(foo)
	SER_TABLE_TABLE_SELF_CUSTOM(ser, des)
	SER_TABLE_TABLE_MEMBER_DEFAULT(i)
	SER_TABLE_TABLE_MEMBER_DEFAULT(d)
	SER_TABLE_TABLE_MEMBER_DEFAULT(c)
SER_TABLE_TABLE_END

unsigned int MCFMain(){
	foo f1 = { { 1, 4 }, 2.0, '3' };
	foo f2;

	vector<unsigned char> deq;
	Serializer<foo> sr;
	auto itw = back_inserter(deq);
	sr.Serialize(itw, f1);

	auto itr = deq.cbegin();
	sr.Unserialize(f2, itr, deq.end());
	printf("%d %d %f %c\n", f2.i[0], f2.i[1], f2.d, f2.c);

	return 0;
}
