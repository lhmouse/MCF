#include <MCF/StdMCF.hpp>
#include <MCF/Serialization/VarIntEx.hpp>
#include <iterator>
#include <cstdio>
using namespace std;
using namespace MCF;

unsigned int MCFMain(){
	VarIntEx<unsigned int> vi(0x12345678);

	unsigned char buf[vi.MAX_SERIALIZED_SIZE];
	auto p = buf;
	vi.Serialize(p);
	for(auto r = buf; r != p; ++r){
		printf("%02hhX ", *r);
	}
	putchar('\n');

	return 0;
}
